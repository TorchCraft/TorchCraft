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

Client::Client(Connection conn) : conn_(std::move(conn)) {}

bool Client::init(std::string& dest, Options opts) {
  std::ostringstream ss;
  ss << "protocol=16";
  if (!opts.initialMap.empty()) {
    ss << ",map=" << opts.initialMap;
  }
  if (opts.windowSize[0] >= 0) {
    ss << ",window_size=" << opts.windowSize[0] << " " << opts.windowSize[1];
  }
  if (opts.windowPos[0] >= 0) {
    ss << ",window_pos=" << opts.windowPos[0] << " " << opts.windowPos[1];
  }
  ss << ",micro_mode=" << (opts.microBattles ? "true" : "false");

  clearError();
  if (!conn_.send(ss.str())) {
    std::stringstream ss;
    ss << "Error sending init request: " << conn_.errmsg() << " ("
       << conn_.errnum() << ")";
    error_ = ss.str();
    return false;
  }

  if (!conn_.receive(dest)) {
    std::stringstream ss;
    ss << "Error receiving init reply: " << conn_.errmsg() << " ("
       << conn_.errnum() << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = false;
  return true;
}

bool Client::send(const std::string& msg) {
  clearError();
  if (sent_) {
    error_ = "Attempt to perform successive sends";
    return false;
  }

  if (!conn_.send(msg)) {
    std::stringstream ss;
    ss << "Error sending request: " << conn_.errmsg() << " (" << conn_.errnum()
       << ")";
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
  if (!conn_.poll(30000)) {
    error_ = "Timeout while waiting for server";
    return false;
  }
  if (!conn_.receive(dest)) {
    std::stringstream ss;
    ss << "Error receiving reply: " << conn_.errmsg() << " (" << conn_.errnum()
       << ")";
    error_ = ss.str();
    return false;
  }
  sent_ = false;
  return true;
}

} // namespace client
