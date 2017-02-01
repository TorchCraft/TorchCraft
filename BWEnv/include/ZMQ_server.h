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

#include <czmq.h>
#include "controller.h"
#include "messages_generated.h"

class Controller;

class ZMQ_server
{
  static const int protocol_version = 17;
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
  void sendHandshake(const TorchCraft::HandshakeServerT* handshake);
  void sendFrame(const TorchCraft::FrameT* frame);
  void sendPlayerLeft(const TorchCraft::PlayerLeftT *pl);
  void sendEndGame(const TorchCraft::EndGameT *endgame);
  void sendError(const TorchCraft::ErrorT *error);
  void receiveMessage();
  void handleReconnect(const TorchCraft::HandshakeClient* handshake);
  void handleCommands(const TorchCraft::Commands* commands);
  int getPort();
};

#endif // TORCHCRAFT_ZMQ_H_
