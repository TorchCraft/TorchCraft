/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <BWAPI.h>

#include <winsock2.h>
#include "zmq_server.h"
#include "controller.h"
#include <utils.h>
#include <thread>
#include <chrono>

using namespace std;

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
    DWORD reuse = 1;
    while (success == -1) {
      stringstream url;
      url << "tcp://*:" << this->port;
      zsock_destroy(&this->server_sock);
      this->server_sock = zsock_new(ZMQ_REP);
      if (setsockopt(zsock_fd(this->server_sock), SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) != 0) {
        std::cout << "SO_REUSEADDR setsockopt failed with " << WSAGetLastError() << std::endl;
      }
      success = zsock_bind(this->server_sock, url.str().c_str());
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  if (this->server_sock == nullptr) {
    throw exception("No more free ports for ZMQ server socket.");
  }

  this->server_sock_connected = true;

  char* welcome_message;
  if (zsock_recv(server_sock, "s", &welcome_message) != 0) {
    throw exception("ZMQ_server::connect(): zsock_recv failed.");
  }

  if (strlen(welcome_message) == 0) {
    zsock_send(this->server_sock, "s", "{}");
    zstr_free(&welcome_message);
    if (zsock_recv(server_sock, "s", &welcome_message) != 0) {
      throw exception("ZMQ_server::connect(): zsock_recv failed.");
    }
  }

  Utils::bwlog(controller->output_log, "From Client: %s", welcome_message);

  // expect welcome message
  if (!checkForWelcomeMessage(welcome_message)) {
    zstr_free(&welcome_message);
    throw logic_error(string("Wrong welcome message, got: ")
      + welcome_message);
  }
  zstr_free(&welcome_message);
}

ZMQ_server::~ZMQ_server()
{
  this->close();
}

bool ZMQ_server::checkProtocolMessage(const char* msg) {
  // Returns true if protocol message with correct version
  // Raises logic_error if protocol message with wrong version
  // Returns false if not protocol message
  const char* s_protocol = "protocol=";
  if (!strncmp(msg, s_protocol, strlen(s_protocol))) {
    if (ZMQ_server::protocol_version !=
      atoi(msg + strlen(s_protocol))) {
      throw logic_error(string("Wrong protocol version: ") + msg);
    }
    return true;
  }
  else {
    return false;
  }
}

std::string ZMQ_server::checkInitialMap(const char* msg)
{
  auto s = std::string("");
  const char* s_map = "map=";
  auto found_index = strstr(msg, s_map);
  // assuming string ends with map_path\0 or ,
  if (found_index != nullptr){
    s = std::string(found_index + 4);
    auto possible_comma = s.find(',');
    if (possible_comma != std::string::npos)
    {
      s = s.substr(0, possible_comma);
    }
  }
  return s;
}

std::pair<int, int> ZMQ_server::checkWindowSize(const char* msg)
{
  auto sizes = std::pair<int, int>(-1, -1);
  auto s = std::string("");
  const char* s_ws = "window_size=";
  auto found_index = strstr(msg, s_ws);
  // assuming string ends with map_path\0 or ,
  if (found_index != nullptr){
    s = std::string(found_index + 12);
    auto possible_comma = s.find(',');

    if (possible_comma != std::string::npos)
    {
      s = s.substr(0, possible_comma);
    }

    std::istringstream stream(s);
    stream >> sizes.first >> sizes.second;
  }

  return sizes;
}

std::pair<int, int> ZMQ_server::checkWindowPos(const char* msg)
{
  auto pos = std::pair<int, int>(-1, -1);
  auto s = std::string("");
  const char* s_wp = "window_pos=";
  auto found_index = strstr(msg, s_wp);
  // assuming string ends with map_path\0 or ,
  if (found_index != nullptr){
    s = std::string(found_index + 11);
    auto possible_comma = s.find(',');

    if (possible_comma != std::string::npos)
    {
      s = s.substr(0, possible_comma);
    }

    std::istringstream stream(s);
    stream >> pos.first >> pos.second;
  }

  return pos;
}

bool ZMQ_server::checkMode(const char* msg)
{
  // Sets controller.micro_mode, because we prefer to give
  // control to the Torch part instead of enforcing that
  // BWAPI::Broodwar->getGameType() == BWAPI::GameTypes::Use_Map_Settings
  // for micro_mode to be true.

  auto s = string(msg);
  auto i = s.find("micro_mode=");
  i = i + 11;
  auto sf = s.substr(i, 4);
  if (sf == "true")
    return true;
  return false;
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

void ZMQ_server::packMessage(const string &str)
{
  /* concatenating the elements in a Lua table */
  if (!this->server_sock_connected) return;
  this->sbuf << str << ",";
}

void ZMQ_server::sendMessage()
{
  /* if not yet connected, do nothing */
  if (!this->server_sock_connected) return;

  string str = "{" + this->sbuf.str() + "}";
  this->sbuf.str("");

  // Message is sent as bytestream to tell zmq to not ignore null
  // bytes (for image)
  if (zsock_send(this->server_sock,
    "b", reinterpret_cast<const byte*>(str.c_str()), str.size()) != 0) {
    throw exception("ZMQ_server::sendMessage(): zsock_send failed.");
  }
}

void ZMQ_server::receiveMessage()
{
  /* if not yet connected, do nothing */
  if (!this->server_sock_connected) return;

  char *message;
  if (zsock_recv(this->server_sock, "s", &message) != 0) {
    throw exception("ZMQ_server::receiveMessage(): zsock_recv failed.");
  }

  // if message is welcome message it means it reconnected
  if (!checkForWelcomeMessage(message))
    this->executeTokenize(message);
  zstr_free(&message);
}

bool ZMQ_server::checkForWelcomeMessage(const char* msg) {
  if (!checkProtocolMessage(msg))
    return false;

  auto map_path = checkInitialMap(msg);
  if (map_path.length() > 0)
    controller->setMap(map_path);

  auto sizes = checkWindowSize(msg);
  if (!(sizes.first < 0 || sizes.second < 0))
    controller->setWindowSize(sizes);

  auto pos = checkWindowPos(msg);
  if (!(pos.first < 0 || pos.second < 0))
    controller->setWindowPos(pos);

  controller->micro_mode = checkMode(msg);

  // if we aren't in client mode it means that game has started already
  // so we can go ahead with handshake
  if (!controller->is_client)
    controller->setupHandshake();

  return true;
}

void ZMQ_server::executeTokenize(char* message)
{
  Utils::bwlog(controller->output_log, "(%d) Received: %s", BWAPI::Broodwar->getFrameCount(), message);

  char* saveptr;
  char* token = strtok_s(message, ":", &saveptr);
  int commandCount = 0;
  char* commands[ZMQ_server::max_commands];

  while (token != nullptr && commandCount < ZMQ_server::max_commands)
  {
    commands[commandCount] = token;
    commandCount++;
    token = strtok_s(nullptr, ":", &saveptr);
  }

  // tokenize the arguments
  std::vector<int> args;
  args.reserve(6);
  std::string str;
  for (int i = 0; i < commandCount; i++)
  {
    if (strlen(commands[i]) == 0) continue;
    int command = atoi(strtok_s(commands[i], ",", &saveptr));
    auto toConv = strtok_s(nullptr, ",", &saveptr);
    if (command == Commands::SET_MAP)
    {
      str = std::string(toConv);
      args.push_back(0); // doesn't matter, as we are going to ignore this.
    }
    else
    {
      while (toConv != nullptr) {
        args.push_back(atoi(toConv));
        toConv = strtok_s(nullptr, ",", &saveptr);
      }
    }
    try {
      controller->handleCommand(command, args, str);
    }
    catch (std::runtime_error& e) {
      Utils::bwlog(controller->output_log, "Exception : %s", e.what());
    }
    args.clear();
    str.clear();
  }
}

int ZMQ_server::getPort()
{
  return port;
}