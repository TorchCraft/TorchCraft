/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>
#include <sstream>

#include "client.h"
#include "constants.h"
#include "state.h"

#include "BWEnv/fbs/messages_generated.h"

namespace {

void buildHandshakeMessage(
    flatbuffers::FlatBufferBuilder& fbb,
    const client::Client::Options& opts) {
  TorchCraft::HandshakeClientT hsc;
  hsc.protocol = 17;
  hsc.map = opts.initial_map;
  if (opts.window_size[0] >= 0) {
    hsc.window_size.reset(
        new TorchCraft::Vec2(opts.window_size[0], opts.window_size[1]));
  }
  if (opts.window_pos[0] >= 0) {
    hsc.window_pos.reset(
        new TorchCraft::Vec2(opts.window_pos[0], opts.window_pos[1]));
  }
  hsc.micro_mode = opts.micro_battles;

  auto payload = TorchCraft::HandshakeClient::Pack(fbb, &hsc);
  auto root = TorchCraft::CreateMessage(
      fbb, TorchCraft::Any::HandshakeClient, payload.Union());
  TorchCraft::FinishMessageBuffer(fbb, root);
}

void buildCommandMessage(
    flatbuffers::FlatBufferBuilder& fbb,
    const std::vector<client::Client::Command>& commands) {
  std::vector<flatbuffers::Offset<TorchCraft::Command>> offsets;
  for (auto comm : commands) {
    offsets.push_back(TorchCraft::CreateCommandDirect(
        fbb, comm.code, &comm.args, comm.str.c_str()));
  }

  auto payload = TorchCraft::CreateCommandsDirect(fbb, &offsets);
  auto root = TorchCraft::CreateMessage(
      fbb, TorchCraft::Any::Commands, payload.Union());
  TorchCraft::FinishMessageBuffer(fbb, root);
}

} // namespace

namespace client {

void init() {
  client::BW::data::init();
}

//============================= LIFECYCLE ====================================

Client::Client() : state_(new State()) {}

Client::~Client() {
  state_->decref();
}

//============================= OPERATIONS ===================================

bool Client::connect(const std::string& hostname, int port,
    int send_timeout_ms /* = -1 */, int receive_timeout_ms /* = -1 */) {
  clearError();
  if (conn_) {
    error_ = "Active connection present";
    return false;
  }

  try {
    conn_.reset(new Connection(hostname, port, send_timeout_ms,
      receive_timeout_ms));
  } catch (zmq::error_t& e) {
    error_ = e.what();
    return false;
  }

  state_->reset();
  sent_ = false;
  return true;
} // connect

bool Client::close() {
  clearError();
  if (!conn_) {
    error_ = "No active connection";
    return false;
  }
  conn_.reset();
  return true;
} // close

bool Client::init(std::vector<std::string>& updates, const Options& opts) {
  flatbuffers::FlatBufferBuilder fbb;
  buildHandshakeMessage(fbb, opts);

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
  if (!TorchCraft::VerifyMessageBuffer(verifier)) {
    error_ = "Error parsing init reply";
    return false;
  }
  auto msg = TorchCraft::GetMessage(reply.data());
  if (msg->msg_type() != TorchCraft::Any::HandshakeServer) {
    error_ = std::string(
                 "Error parsing init reply: expected HandshakeServer, got ") +
        TorchCraft::EnumNameAny(msg->msg_type());
    return false;
  }
  if (!TorchCraft::VerifyAny(
          verifier, msg->msg(), TorchCraft::Any::HandshakeServer)) {
    error_ = "Error parsing init reply";
    return false;
  }

  state_->setMicroBattles(opts.micro_battles);
  state_->setOnlyConsiderTypes(opts.only_consider_types);
  updates = state_->update(
      reinterpret_cast<const TorchCraft::HandshakeServer*>(msg->msg()));
  return true;
} // init

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
  buildCommandMessage(fbb, commands);

  if (!conn_->send(fbb.GetBufferPointer(), fbb.GetSize())) {
    std::stringstream ss;
    ss << "Error sending request: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = true;
  return true;
} // send

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
  if (!TorchCraft::VerifyMessageBuffer(verifier)) {
    error_ = "Error parsing reply";
    return false;
  }
  auto msg = TorchCraft::GetMessage(reply.data());
  if (!TorchCraft::VerifyAny(verifier, msg->msg(), msg->msg_type())) {
    error_ = "Error parsing reply";
    return false;
  }

  switch (msg->msg_type()) {
    case TorchCraft::Any::Frame:
      updates = state_->update(
          reinterpret_cast<const TorchCraft::Frame*>(msg->msg()));
      break;
    case TorchCraft::Any::EndGame:
      updates = state_->update(
          reinterpret_cast<const TorchCraft::EndGame*>(msg->msg()));
      break;
    case TorchCraft::Any::HandshakeServer:
      updates = state_->update(
          reinterpret_cast<const TorchCraft::HandshakeServer*>(msg->msg()));
      break;
    // TODO These message types were not explicitly handled in the Lua version
    case TorchCraft::Any::PlayerLeft:
      std::cerr << "[Warning] Unhandled message from server: "
                << TorchCraft::EnumNameAny(msg->msg_type()) << "(player_left=\""
                << reinterpret_cast<const TorchCraft::PlayerLeft*>(msg->msg())
                       ->player_left()
                       ->str()
                << "\")" << std::endl;
      break;
    case TorchCraft::Any::Error:
      std::cerr << "[Warning] Unhandled message from server: "
                << TorchCraft::EnumNameAny(msg->msg_type()) << "(message=\""
                << reinterpret_cast<const TorchCraft::Error*>(msg->msg())
                       ->message()
                       ->str()
                << "\"" << std::endl;
      break;
    default:
      error_ = std::string("Error parsing reply: cannot handle message: ") +
          TorchCraft::EnumNameAny(msg->msg_type());
      return false;
  }
  return true;
}

} // namespace client
