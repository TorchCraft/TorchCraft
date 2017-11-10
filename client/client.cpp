/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>
#include <mutex>
#include <random>
#include <sstream>

#include "client.h"
#include "connection.h"
#include "state.h"
#include "torchcraft_generated.h"

namespace {

std::string makeUid(size_t len = 6) {
  static std::mt19937 rng = std::mt19937(std::random_device()());

  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::uniform_int_distribution<int> dis(0, sizeof(alphanum) - 1);
  std::string s(len, 0);
  for (size_t i = 0; i < len; i++) {
    s[i] = alphanum[dis(rng)];
  }
  return s;
}

void buildHandshakeMessage(
    flatbuffers::FlatBufferBuilder& fbb,
    const torchcraft::Client::Options& opts,
    const std::string* uid = nullptr) {
  torchcraft::fbs::HandshakeClientT hsc;
  hsc.protocol = 22;
  hsc.map = opts.initial_map;
  if (opts.window_size[0] >= 0) {
    hsc.window_size.reset(
        new torchcraft::fbs::Vec2(opts.window_size[0], opts.window_size[1]));
  }
  if (opts.window_pos[0] >= 0) {
    hsc.window_pos.reset(
        new torchcraft::fbs::Vec2(opts.window_pos[0], opts.window_pos[1]));
  }
  hsc.micro_mode = opts.micro_battles;

  auto payload = torchcraft::fbs::HandshakeClient::Pack(fbb, &hsc);
  auto root = torchcraft::fbs::CreateMessageDirect(
      fbb,
      torchcraft::fbs::AnyMessage::HandshakeClient,
      payload.Union(),
      uid ? uid->c_str() : nullptr);
  torchcraft::fbs::FinishMessageBuffer(fbb, root);
}

void buildCommandMessage(
    flatbuffers::FlatBufferBuilder& fbb,
    const std::vector<torchcraft::Client::Command>& commands,
    const std::string* uid = nullptr) {
  std::vector<flatbuffers::Offset<torchcraft::fbs::Command>> offsets;
  for (auto comm : commands) {
    offsets.push_back(torchcraft::fbs::CreateCommandDirect(
        fbb, comm.code, &comm.args, comm.str.c_str()));
  }

  auto payload = torchcraft::fbs::CreateCommandsDirect(fbb, &offsets);
  auto root = torchcraft::fbs::CreateMessageDirect(
      fbb,
      torchcraft::fbs::AnyMessage::Commands,
      payload.Union(),
      uid ? uid->c_str() : nullptr);
  torchcraft::fbs::FinishMessageBuffer(fbb, root);
}

std::once_flag initFlag; // For protecting doInit(); see init() below
void doInit() {
  torchcraft::BW::data::init();
}

} // namespace

namespace torchcraft {

void init() {
  std::call_once(initFlag, doInit);
}

//============================= LIFECYCLE ====================================

Client::Client() : state_(new State()) {}

Client::~Client() {
  state_->decref();
}

//============================= OPERATIONS ===================================

bool Client::connect(
    const std::string& hostname,
    int port,
    int timeoutMs /* = -1 */) {
  clearError();
  if (conn_) {
    error_ = "Active connection present";
    return false;
  }

  try {
    conn_.reset(new Connection(hostname, port, timeoutMs));
    uid_ = makeUid();
  } catch (zmq::error_t& e) {
    error_ = e.what();
    return false;
  }

  state_->reset();
  sent_ = false;
  return true;
}

bool Client::close() {
  clearError();
  if (!conn_) {
    error_ = "No active connection";
    return false;
  }
  conn_.reset();
  return true;
}

bool Client::init(std::vector<std::string>& updates, const Options& opts) {
  flatbuffers::FlatBufferBuilder fbb;
  buildHandshakeMessage(fbb, opts, &uid_);

  clearError();
  if (!conn_) {
    error_ = "No active connection";
    return false;
  }

  if (!conn_->send(fbb.GetBufferPointer(), fbb.GetSize())) {
    std::stringstream ss;
    ss << "Error sending init request: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }

  std::vector<uint8_t> reply;
  if (!conn_->receive(reply)) {
    std::stringstream ss;
    ss << "Error receiving init reply: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = false;

  flatbuffers::Verifier verifier(reply.data(), reply.size());
  if (!torchcraft::fbs::VerifyMessageBuffer(verifier)) {
    error_ = "Error parsing init reply";
    return false;
  }
  auto msg = torchcraft::fbs::GetMessage(reply.data());
  if (msg->msg_type() != torchcraft::fbs::AnyMessage::HandshakeServer) {
    error_ = std::string(
                 "Error parsing init reply: expected HandshakeServer, got ") +
        torchcraft::fbs::EnumNameAnyMessage(msg->msg_type());
    return false;
  }
  if (!torchcraft::fbs::VerifyAnyMessage(
          verifier, msg->msg(), torchcraft::fbs::AnyMessage::HandshakeServer)) {
    error_ = "Error parsing init reply";
    return false;
  }

  state_->setMicroBattles(opts.micro_battles);
  state_->setOnlyConsiderTypes(opts.only_consider_types);
  updates = state_->update(
      reinterpret_cast<const torchcraft::fbs::HandshakeServer*>(msg->msg()));
  return true;
}

bool Client::send(const std::vector<Command>& commands) {
  clearError();
  if (sent_) {
    error_ = "Attempt to perform successive sends";
    return false;
  }

  if (!conn_) {
    error_ = "No active connection";
    return false;
  }

  flatbuffers::FlatBufferBuilder fbb;
  buildCommandMessage(fbb, commands, &uid_);

  if (!conn_->send(fbb.GetBufferPointer(), fbb.GetSize())) {
    std::stringstream ss;
    ss << "Error sending request: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }

  sent_ = true;
  lastCommands_ = commands;
  lastCommandsStatus_.clear();
  return true;
}

bool Client::receive(std::vector<std::string>& updates) {
  if (!sent_) {
    send(std::vector<Command>());
  }

  clearError();
  if (!conn_) {
    error_ = "No active connection";
    return false;
  }

  std::vector<uint8_t> reply;
  if (!conn_->receive(reply)) {
    std::stringstream ss;
    ss << "Error receiving reply: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = false;

  flatbuffers::Verifier verifier(reply.data(), reply.size());
  if (!torchcraft::fbs::VerifyMessageBuffer(verifier)) {
    error_ = "Error parsing reply";
    return false;
  }
  auto msg = torchcraft::fbs::GetMessage(reply.data());
  if (!torchcraft::fbs::VerifyAnyMessage(verifier, msg->msg(), msg->msg_type())) {
    error_ = "Error parsing reply";
    return false;
  }

  switch (msg->msg_type()) {
    case torchcraft::fbs::AnyMessage::FrameState: {
      auto frameMsg =
          reinterpret_cast<const torchcraft::fbs::FrameState*>(msg->msg());
      if (flatbuffers::IsFieldPresent(
              frameMsg, torchcraft::fbs::FrameState::VT_COMMANDS_STATUS)) {
        auto& cs = *frameMsg->commands_status();
        lastCommandsStatus_.resize(cs.size());
        for (size_t i = 0; i < cs.size(); i++) {
          lastCommandsStatus_[i] = cs[i];
        }
      }
      updates = state_->update(frameMsg);
      break;
    }
    case torchcraft::fbs::AnyMessage::EndGame:
      updates = state_->update(
          reinterpret_cast<const torchcraft::fbs::EndGame*>(msg->msg()));
      break;
    case torchcraft::fbs::AnyMessage::HandshakeServer:
      updates = state_->update(
          reinterpret_cast<const torchcraft::fbs::HandshakeServer*>(
              msg->msg()));
      break;
    // TODO These message types were not explicitly handled in the Lua version
    case torchcraft::fbs::AnyMessage::PlayerLeft: {
      auto left =
          reinterpret_cast<const torchcraft::fbs::PlayerLeft*>(msg->msg())
              ->player_left();
      std::cerr << "[Warning] Unhandled message from server: "
                << torchcraft::fbs::EnumNameAnyMessage(msg->msg_type())
                << "(player_left=\"" << (left ? left->str() : "(null)") << "\")"
                << std::endl;
      updates = state_->update(
          reinterpret_cast<const torchcraft::fbs::PlayerLeft*>(msg->msg()));
      break;
    }
    case torchcraft::fbs::AnyMessage::Error: {
      auto text = reinterpret_cast<const torchcraft::fbs::Error*>(msg->msg())
                      ->message();
      std::cerr << "[Warning] Unhandled message from server: "
                << torchcraft::fbs::EnumNameAnyMessage(msg->msg_type())
                << "(message=\"" << (text ? text->str() : "(null)") << "\""
                << std::endl;
      updates = state_->update(
          reinterpret_cast<const torchcraft::fbs::Error*>(msg->msg()));
      break;
    }
    default:
      error_ = std::string("Error parsing reply: cannot handle message: ") +
          torchcraft::fbs::EnumNameAnyMessage(msg->msg_type());
      return false;
  }
  return true;
}

bool Client::poll(long timeout) {
  clearError();
  if (!conn_) {
    error_ = "No active connection";
    return false;
  }

  if (!conn_->poll(timeout)) {
    std::stringstream ss;
    ss << "Error during poll: " << conn_->errmsg() << " (" << conn_->errnum()
       << ")";
    error_ = ss.str();
    return false;
  }

  return true;
}

} // namespace torchcraft
