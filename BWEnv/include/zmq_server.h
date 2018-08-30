/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef TORCHCRAFT_ZMQ_H_
#define TORCHCRAFT_ZMQ_H_

#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include "zmq.hpp"
#include "controller.h"
#include "messages_generated.h"

class Controller;

class ZMQ_server
{
  static const int protocol_version = 31; // Should match HandshakeClient
  static const int max_commands = 2500; // maximum number of commands per frame
  static const int starting_port = 11111;
  static const int max_instances = 1000;

  Controller *controller;
  std::unique_ptr<zmq::context_t> ctx;
  std::unique_ptr<zmq::socket_t> sock;
  int port = 0;
  std::string file_socket;
public:
  bool server_sock_connected;

  ZMQ_server(Controller *c, int port);
  ZMQ_server(Controller *c, std::string const& file_socket);
  ~ZMQ_server();

  void connect();
  void close();
  void sendHandshake(const torchcraft::fbs::HandshakeServerT* handshake);
  void sendFrame(
    const flatbuffers::Offset<torchcraft::fbs::StateUpdate>& stateUpdateOffset,
    flatbuffers::FlatBufferBuilder& builder);
  void sendPlayerLeft(const torchcraft::fbs::PlayerLeftT *pl);
  void sendEndGame(
    const flatbuffers::Offset<torchcraft::fbs::EndGame>& endGameOffset,
    flatbuffers::FlatBufferBuilder& builder);
  void sendError(const torchcraft::fbs::ErrorT *error);
  bool receiveMessage(int timeoutMs = -1);
  void handleReconnect(const torchcraft::fbs::HandshakeClient* handshake);
  std::vector<int8_t> handleCommands(const torchcraft::fbs::Commands* commands);
  int getPort();
  std::string getFileSocketName();
};

#endif // TORCHCRAFT_ZMQ_H_
