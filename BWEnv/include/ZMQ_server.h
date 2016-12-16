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

#include <sstream>

#include <czmq.h>
#include "controller.h"

class Controller;

class ZMQ_server
{
  static const int protocol_version = 16;
  static const int max_commands = 400; // maximum number of commands per frame
  static const int starting_port = 11111;
  static const int max_instances = 1000;

  Controller *controller;
  zsock_t* server_sock = nullptr;
  std::stringstream sbuf; /** stringstream buffers for zmq */
  int port = 0;
public:
  bool server_sock_connected;

  explicit ZMQ_server(Controller *c, int port);
  ~ZMQ_server();

  void connect();
  bool checkForWelcomeMessage(const char*);
  bool checkProtocolMessage(const char*);
  std::string checkInitialMap(const char* msg);
  std::pair<int, int> checkWindowSize(const char* msg);
  std::pair<int, int> checkWindowPos(const char* msg);
  bool checkMode(const char* msg);
  void close();
  void packMessage(const std::string&);
  void sendMessage();
  void receiveMessage();
  void executeTokenize(char*);
  int getPort();
};

#endif // TORCHCRAFT_ZMQ_H_
