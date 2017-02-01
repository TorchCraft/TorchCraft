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

#include "connection.h"

namespace client {

Connection::Connection(const std::string& hostname, int port)
    : sock_(ctx_, zmq::socket_type::req) {
  std::ostringstream ss;
  ss << "tcp://" << hostname << ":" << port;
  sock_.connect(ss.str());
}

Connection::Connection(Connection&& conn)
    : ctx_(std::move(conn.ctx_)), sock_(std::move(conn.sock_)) {}

bool Connection::send(const std::string& data) {
  clearError();
  try {
    sock_.send(data.begin(), data.end());
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }
  return true;
}

bool Connection::send(const void* buf, size_t len) {
  clearError();
  try {
    sock_.send(buf, len);
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }
  return true;
}

bool Connection::receive(std::string& dest) {
  clearError();
  try {
    sock_.recv(&recvmsg_);
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }

  dest.assign(recvmsg_.data<char>(), recvmsg_.size());
  return true;
}

bool Connection::receive(std::vector<uint8_t>& dest) {
  clearError();
  try {
    sock_.recv(&recvmsg_);
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }

  auto d = recvmsg_.data<unsigned char>();
  dest.assign(d, d + recvmsg_.size());
  return true;
}

bool Connection::poll(int timeout) {
  short mask = ZMQ_POLLIN;
  zmq::pollitem_t items[] = {{sock_, 0, mask, 0}};

  clearError();
  try {
    zmq::poll(items, 1, timeout);
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }

  return items[0].revents & mask;
}

void Connection::clearError() {
  errnum_ = 0;
  errmsg_.clear();
}

} // namespace client
