/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <BWAPI.h>
#include <BWAPI/Client.h>
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "zmq_server.h"
#include "controller.h"
#include <utils.h>
#include <thread>
#include <chrono>

using namespace std;

namespace {

template<typename T>
void sendFBObject(zsock_t* sock, const T* obj) {
  flatbuffers::FlatBufferBuilder fbb;
  auto payload = T::TableType::Pack(fbb, obj);
  auto root = torchcraft::fbs::CreateMessage(
      fbb, torchcraft::fbs::AnyTraits<typename T::TableType>::enum_value, payload.Union());
  torchcraft::fbs::FinishMessageBuffer(fbb, root);

  if (zsock_send(sock, "b", fbb.GetBufferPointer(), fbb.GetSize()) != 0) {
    throw runtime_error("ZMQ_server::send(): zmq_send failed.");
  }
}

} // namespace

ZMQ_server::ZMQ_server(Controller *c, int port)
{
  server_sock_connected = false;
  controller = c;
  this->port = port;
}

void ZMQ_server::connect()
{
  if (this->server_sock_connected) return;

  zsys_shutdown(); /* reinitialize zsys*/

  if (this->port == 0) {
    for (int port = ZMQ_server::starting_port;
      port < ZMQ_server::starting_port
      + ZMQ_server::max_instances;
    port++) {
      stringstream url;
      url << "tcp://*:" << port;
      this->server_sock = zsock_new_rep(url.str().c_str());
      if (this->server_sock != nullptr) {
        this->port = port;
        break;
      }
    }
  }
  else {
    int success = -1;
#ifdef _WIN32
    DWORD reuse = 1;
#endif
    while (success == -1) {
      stringstream url;
      url << "tcp://*:" << this->port;
      zsock_destroy(&this->server_sock);
      this->server_sock = zsock_new(ZMQ_REP);
#ifdef _WIN32
      if (setsockopt(zsock_fd(this->server_sock), SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) != 0) {
        std::cout << "SO_REUSEADDR setsockopt failed with " << WSAGetLastError() << std::endl;
      }
#endif
      success = zsock_bind(this->server_sock, "%s", url.str().c_str());
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  if (this->server_sock == nullptr) {
    throw runtime_error("No more free ports for ZMQ server socket.");
  }

  this->server_sock_connected = true;

  zchunk_t *chunk;
  if (zsock_recv(this->server_sock, "c", &chunk) != 0) {
    throw runtime_error("ZMQ_server::connect(): zmq_recv failed.");
  }

  if (zchunk_size(chunk) == 0) {
    // Retry
    torchcraft::fbs::ErrorT err;
    sendError(&err);
    zchunk_destroy(&chunk);
    if (zsock_recv(this->server_sock, "c", &chunk) != 0) {
      throw runtime_error("ZMQ_server::connect(): zsock_recv failed.");
    }
  }

  uint8_t* data = zchunk_data(chunk);
  size_t size = zchunk_size(chunk);
  flatbuffers::Verifier verifier(data, size);
  if (!torchcraft::fbs::VerifyMessageBuffer(verifier)) {
    zchunk_destroy(&chunk);
    throw runtime_error("ZMQ_server::connect(): invalid message.");
  }

  auto msg = torchcraft::fbs::GetMessage(data);
  if (msg->msg_type() == torchcraft::fbs::Any::HandshakeClient) {
    if (!torchcraft::fbs::VerifyAny(
            verifier, msg->msg(), torchcraft::fbs::Any::HandshakeClient)) {
      zchunk_destroy(&chunk);
      throw runtime_error("ZMQ_server::connect(): invalid message.");
    }
    handleReconnect(
        reinterpret_cast<const torchcraft::fbs::HandshakeClient*>(msg->msg()));
  } else {
    zchunk_destroy(&chunk);
    throw logic_error(
        string("ZMQ_server::connect(): cannot handle message: ") +
        torchcraft::fbs::EnumNameAny(msg->msg_type()));
  }

  zchunk_destroy(&chunk);
}

ZMQ_server::~ZMQ_server()
{
  this->close();
}

void ZMQ_server::close()
{
  if (!this->server_sock_connected) return;

  // Called when the game ends
  zsock_destroy(&this->server_sock);
  this->server_sock = nullptr;
  zsys_shutdown();

  Utils::bwlog(controller->output_log, "socket: %d", server_sock);
  Utils::bwlog(controller->output_log, "after zsock destroy");

  this->server_sock_connected = false;
}

void ZMQ_server::sendHandshake(const torchcraft::fbs::HandshakeServerT* handshake) {
  sendFBObject(this->server_sock, handshake);
}

void ZMQ_server::sendFrame(const torchcraft::fbs::FrameT* frame) {
  sendFBObject(this->server_sock, frame);
}

void ZMQ_server::sendPlayerLeft(const torchcraft::fbs::PlayerLeftT* pl) {
  sendFBObject(this->server_sock, pl);
}

void ZMQ_server::sendEndGame(const torchcraft::fbs::EndGameT* endgame) {
  sendFBObject(this->server_sock, endgame);
}

void ZMQ_server::sendError(const torchcraft::fbs::ErrorT* error) {
  sendFBObject(this->server_sock, error);
}

void ZMQ_server::receiveMessage()
{
  /* if not yet connected, do nothing */
  if (!this->server_sock_connected) return;

  zchunk_t *chunk;
  if (zsock_recv(this->server_sock, "c", &chunk) != 0) {
    throw runtime_error("ZMQ_server::receiveMessage(): zmq_recv failed.");
  }

  uint8_t *data = zchunk_data(chunk);
  size_t size = zchunk_size(chunk);
  flatbuffers::Verifier verifier(data, size);
  if (!torchcraft::fbs::VerifyMessageBuffer(verifier)) {
    zchunk_destroy(&chunk);
    throw runtime_error("ZMQ_server::receiveMessage(): invalid message.");
  }

  auto msg = torchcraft::fbs::GetMessage(data);
  if (!torchcraft::fbs::VerifyAny(verifier, msg->msg(), msg->msg_type())) {
    zchunk_destroy(&chunk);
    throw runtime_error("ZMQ_server::receiveMessage(): invalid message.");
  }

  switch (msg->msg_type()) {
    case torchcraft::fbs::Any::HandshakeClient: // reconnection
      handleReconnect(
          reinterpret_cast<const torchcraft::fbs::HandshakeClient*>(msg->msg()));
      break;
    case torchcraft::fbs::Any::Commands: {
    auto status = handleCommands(reinterpret_cast<const torchcraft::fbs::Commands*>(msg->msg()));
      controller->setCommandsStatus(status);
      break;
  }
    default:
      zchunk_destroy(&chunk);
      throw runtime_error(
          string("ZMQ_server::receiveMessage(): cannot handle message: ") +
          torchcraft::fbs::EnumNameAny(msg->msg_type()));
  }

  zchunk_destroy(&chunk);
}

void ZMQ_server::handleReconnect(const torchcraft::fbs::HandshakeClient* handshake) {
  if (handshake->protocol() != ZMQ_server::protocol_version) {
    throw logic_error(
        string("Wrong protocol version: ") + to_string(handshake->protocol()));
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeClient::VT_MAP)) {
    controller->setMap(handshake->map()->str());
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeClient::VT_WINDOW_SIZE)) {
    controller->setWindowSize(pair<int, int>(
        handshake->window_size()->x(), handshake->window_size()->y()));
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeClient::VT_WINDOW_POS)) {
    controller->setWindowPos(pair<int, int>(
        handshake->window_size()->x(), handshake->window_size()->y()));
  }
  controller->micro_mode = handshake->micro_mode();

  // if we aren't in client mode it means that game has started already
  // so we can go ahead with handshake
  if (!controller->is_client || (BWAPI::BWAPIClient.isConnected() && BWAPI::Broodwar->isInGame()))
    controller->setupHandshake();
}

std::vector<int8_t> ZMQ_server::handleCommands(const torchcraft::fbs::Commands* comms) {
  // Results: 0 = success, error code otherwise
  std::vector<int8_t> results;

  if (!flatbuffers::IsFieldPresent(comms, torchcraft::fbs::Commands::VT_COMMANDS)) {
    return results;
  }
  auto commands = comms->commands();
  if (commands->size() > 0) {
    Utils::bwlog(controller->output_log, "(%d) Received %d commands", BWAPI::Broodwar->getFrameCount(), commands->size());
  }

  int count = 0;
  for (auto comm : *commands) {
    if (count >= ZMQ_server::max_commands) {
      results.insert(results.end(), commands->size() - results.size(), CommandStatus::TOO_MANY_COMMANDS);
      break;
    }
    int8_t res;
    try {
      std::vector<int> args(comm->args()->data(), comm->args()->data() + comm->args()->size());
      res = controller->handleCommand(comm->code(), args, comm->str()->str());
    } catch (std::exception& e) {
      Utils::bwlog(controller->output_log, "Exception : %s", e.what());
      res = CommandStatus::UNKNOWN_ERROR;
      throw;
    }
    results.push_back(res);
    count++;
  }
  return results;
}

int ZMQ_server::getPort()
{
  return port;
}
