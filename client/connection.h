/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "zmq.hpp"

namespace client {

class Connection {
 public:
  Connection(const std::string& hostname, int port);
  Connection(Connection&& conn);

  bool send(const std::string& data);
  bool receive(std::string& dest);
  bool poll(int timeout);

  int errnum() const {
    return errnum_;
  }
  std::string errmsg() const {
    return errmsg_;
  }

 private:
  void clearError();

  zmq::context_t ctx_;
  zmq::socket_t sock_;
  zmq::message_t recvmsg_;
  int errnum_ = 0;
  std::string errmsg_;
};

} // namespace client
