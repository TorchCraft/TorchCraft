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
#include <czmq.h>
#include "controller.h"
#include "messages_generated.h"

class Controller;

class ZMQ_server
{
  static const int protocol_version = 22;
  static const int max_commands = 400; // maximum number of commands per frame
  static const int starting_port = 11111;
  static const int max_instances = 1000;

  Controller *controller;
  zsock_t* server_sock = nullptr;
  int port = 0;
public:
  bool server_sock_connected;

  explicit ZMQ_server(Controller *c, int port);
  ~ZMQ_server();

  void connect();
  void close();
  void sendHandshake(const torchcraft::fbs::HandshakeServerT* handshake);
  void sendFrame(const torchcraft::fbs::FrameT* frame);
  void sendPlayerLeft(const torchcraft::fbs::PlayerLeftT *pl);
  void sendEndGame(const torchcraft::fbs::EndGameT *endgame);
  void sendError(const torchcraft::fbs::ErrorT *error);
  void receiveMessage();
  void handleReconnect(const torchcraft::fbs::HandshakeClient* handshake);
  std::vector<int8_t> handleCommands(const torchcraft::fbs::Commands* commands);
  int getPort();
};

#endif // TORCHCRAFT_ZMQ_H_
