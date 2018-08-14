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

namespace {
const std::string ERRMSG_TIMEOUT_EXCEEDED = "Timeout exceeded";
}

zmq::context_t* Connection::context_ = nullptr;
int Connection::contextRef_ = 0;
std::mutex Connection::contextMutex_;

Connection::Connection(const std::string& hostname, int port, int timeoutMs)
    : ctx_(refContext()),
      sock_(
          std::unique_ptr<zmq::socket_t>(
              new zmq::socket_t(ctx_, zmq::socket_type::req))) {
  std::ostringstream ss;
  ss << "tcp://" << hostname << ":" << port;
  sock_->setsockopt(ZMQ_SNDTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_->setsockopt(ZMQ_RCVTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_->setsockopt(ZMQ_LINGER, 10);
  sock_->setsockopt(ZMQ_IPV6, 1);
  sock_->connect(ss.str());
}

Connection::Connection(const std::string& file_socket, int timeoutMs)
    : ctx_(refContext()),
      sock_(
          std::unique_ptr<zmq::socket_t>(
              new zmq::socket_t(ctx_, zmq::socket_type::req))) {
  std::ostringstream ss;
  ss << "ipc://" << file_socket;
  sock_->setsockopt(ZMQ_SNDTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_->setsockopt(ZMQ_RCVTIMEO, &timeoutMs, sizeof(timeoutMs));
  sock_->setsockopt(ZMQ_LINGER, 10);
  sock_->setsockopt(ZMQ_IPV6, 1);
  sock_->connect(ss.str());
}

Connection::Connection(Connection&& conn)
    : ctx_(refContext()), sock_(std::move(conn.sock_)) {}

Connection::~Connection() {
  // Destroy socket before context
  sock_.reset();
  derefContext();
}

bool Connection::send(const std::string& data) {
  clearError();
  try {
    bool res = sock_->send(data.begin(), data.end());
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
}

bool Connection::send(const void* buf, size_t len) {
  clearError();
  try {
    bool res = sock_->send(buf, len);
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
}

bool Connection::receive(std::vector<uint8_t>& destination) {
  clearError();
  while(true) {
    try {
      bool response = sock_->recv(&recvmsg_);
      if (response) {
        auto data = recvmsg_.data<unsigned char>();
        destination.assign(data, data + recvmsg_.size());
      } else {
        errnum_ = EAGAIN;
        errmsg_ = ERRMSG_TIMEOUT_EXCEEDED;
      }
      return response;
    } catch (zmq::error_t& exception) {
      if (exception.num() == EINTR) {
        continue;
      }
      errnum_ = exception.num();
      errmsg_ = exception.what();
      return false;
    }
  }
}

bool Connection::poll(long timeout) {
  short mask = ZMQ_POLLIN;
  zmq::pollitem_t items[] = {{*sock_, 0, mask, 0}};

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
  } else if (context_ == nullptr) {
    throw std::runtime_error("ZMQ context not initialized");
  }
  return *context_;
}

void Connection::derefContext() {
  std::lock_guard<std::mutex> guard(contextMutex_);
  contextRef_--;
  if (contextRef_ == 0) {
    delete context_;
    context_ = nullptr;
  } else if (contextRef_ < 0) {
    throw std::runtime_error("ZMQ context ref counter < 0");
  }
}

} // namespace torchcraft
