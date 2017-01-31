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

namespace client {

Client::Client(lua_State* L) : L_(L), state_(new State()) {}

Client::~Client() {
  state_->decref();
}

bool Client::connect(const std::string& hostname, int port) {
  clearError();
  if (conn_) {
    error_ = "Active connection present";
    return false;
  }

  try {
    conn_.reset(new Connection(hostname, port));
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

bool Client::init(std::string& dest, Options opts) {
  std::ostringstream ss;
  ss << "protocol=16";
  if (!opts.initial_map.empty()) {
    ss << ",map=" << opts.initial_map;
  }
  if (opts.window_size[0] >= 0) {
    ss << ",window_size=" << opts.window_size[0] << " " << opts.window_size[1];
  }
  if (opts.window_pos[0] >= 0) {
    ss << ",window_pos=" << opts.window_pos[0] << " " << opts.window_pos[1];
  }
  ss << ",micro_mode=" << (opts.micro_battles ? "true" : "false");

  clearError();
  if (!conn_) {
    error_ = "No active connection";
    return false;
  }
  if (!conn_->send(ss.str())) {
    std::stringstream ss;
    ss << "Error sending init request: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }

  std::string reply;
  if (!conn_->receive(reply)) {
    std::stringstream ss;
    ss << "Error receiving init reply: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = false;

  // Retain Lua-parsable message for returning updates to Lua-land
  auto update = state_->update(L_, reply);
  dest.assign(update);
  return true;
}

bool Client::send(const std::string& msg) {
  clearError();
  if (sent_) {
    error_ = "Attempt to perform successive sends";
    return false;
  }

  if (!conn_) {
    error_ = "No active connection";
    return false;
  }
  if (!conn_->send(msg)) {
    std::stringstream ss;
    ss << "Error sending request: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = true;
  return true;
}

bool Client::receive(std::string& dest) {
  if (!sent_) {
    send(std::string());
  }

  clearError();
  if (!conn_) {
    error_ = "No active connection";
    return false;
  }
  if (!conn_->poll(30000)) {
    error_ = "Timeout while waiting for server";
    return false;
  }

  std::string reply;
  if (!conn_->receive(reply)) {
    std::stringstream ss;
    ss << "Error receiving reply: " << conn_->errmsg() << " ("
       << conn_->errnum() << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = false;

  // Retain Lua-parsable message for returning updates to Lua-land
  auto update = state_->update(L_, reply);
  dest.assign(update);

  return true;
}

} // namespace client
