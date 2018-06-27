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
#include <thread>

#include "connection.h"

namespace torchcraft {

static const std::string ERRMSG_TIMEOUT_EXCEEDED = "Timeout exceeded";
zmq::context_t* Connection::context_ = nullptr;
int Connection::contextRef_ = 0;
std::mutex Connection::contextMutex_;

Connection::Connection(const std::string& hostname, int port, int timeoutMs)
    : ctx_(refContext()), sock_(ctx_, zmq::socket_type::req) {
  std::ostringstream ss;
  ss << "tcp://" << hostname << ":" << port;
  sock_.setsockopt(ZMQ_SNDTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_.setsockopt(ZMQ_RCVTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_.setsockopt(ZMQ_LINGER, 10);
  sock_.setsockopt(ZMQ_IPV6, 1);
  sock_.connect(ss.str());
} // Connection

Connection::Connection(const std::string& file_socket, int timeoutMs)
    : ctx_(refContext()), sock_(ctx_, zmq::socket_type::req) {
  std::ostringstream ss;
  ss << "ipc://" << file_socket;
  sock_.setsockopt(ZMQ_SNDTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_.setsockopt(ZMQ_RCVTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_.setsockopt(ZMQ_LINGER, 10);
  sock_.setsockopt(ZMQ_IPV6, 1);
  sock_.connect(ss.str());
} // Connection

Connection::Connection(Connection&& conn)
    : ctx_(refContext()), sock_(std::move(conn.sock_)) {}

Connection::~Connection() {
  derefContext();
}

bool Connection::send(const std::string& data) {
  clearError();
  try {
    bool res = sock_.send(data.begin(), data.end());
    if (!res) {
      errnum_ = EAGAIN;
      errmsg_ = ERRMSG_TIMEOUT_EXCEEDED;
    }
    return res;
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }
} // send

bool Connection::send(const void* buf, size_t len) {
  clearError();
  try {
    bool res = sock_.send(buf, len);
    if (!res) {
      errnum_ = EAGAIN;
      errmsg_ = ERRMSG_TIMEOUT_EXCEEDED;
    }
    return res;
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }
} // send

bool Connection::receive(std::string& dest) {
  clearError();
  try {
    bool res = sock_.recv(&recvmsg_);
    if (res) {
      dest.assign(recvmsg_.data<char>(), recvmsg_.size());
    } else {
      errnum_ = EAGAIN;
      errmsg_ = ERRMSG_TIMEOUT_EXCEEDED;
    }
    return res;
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }
} // receive

bool Connection::receive(std::vector<uint8_t>& dest) {
  clearError();
  try {
    bool res = sock_.recv(&recvmsg_);
    if (res) {
      auto d = recvmsg_.data<unsigned char>();
      dest.assign(d, d + recvmsg_.size());
    } else {
      errnum_ = EAGAIN;
      errmsg_ = ERRMSG_TIMEOUT_EXCEEDED;
    }
    return res;
  } catch (zmq::error_t& e) {
    errnum_ = e.num();
    errmsg_ = e.what();
    return false;
  }
} // receive

bool Connection::poll(long timeout) {
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

zmq::context_t& Connection::refContext() {
  std::lock_guard<std::mutex> guard(contextMutex_);
  contextRef_++;
  if (contextRef_ == 1) {
    if (context_ != nullptr) {
      throw std::runtime_error("ZMQ context already initialized");
    }
    // Quote from http://zeromq.org/area:faq#toc7
    // "The basic heuristic is to allocate 1 I/O thread in the context for every
    // gigabit per second of data that will be sent and received (aggregated).
    // Further, the number of I/O threads should not exceed (number_of_cpu_cores
    // - 1)."
    // We might hit 1 Gbit/s if we run a lot of non-trivial self-play games on a
    // machine
    // with many cores (say, 80 logical ones). So we'll ideally go for 2
    // threads,
    // subject to the number of cores, but fall back to 1 for small systems.
    auto numIoThreads =
        std::max(1, std::min(2, int(std::thread::hardware_concurrency()) - 1));
    context_ = new zmq::context_t(numIoThreads);
  }
  return *context_;
}

void Connection::derefContext() {
  std::lock_guard<std::mutex> guard(contextMutex_);
  contextRef_--;
  if (contextRef_ == 0) {
    delete context_;
    context_ = nullptr;
  }
}

} // namespace torchcraft
