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

namespace torchcraft {

class Connection {
 public:
  /// Creates a new socket and connects it to an endpoint specified
  /// by a hostname and a port. TCP transport protocol is used
  /// for the connection; the full address is thus
  ///     tcp://<hostname>:<port>
  /// @param hostname [in] Hostname part of the TCP address
  /// @param port [in] Port part of the TCP address
  /// @param timeoutMs [in] Send / receive operation timeout in milliseconds
  ///     (default = -1), the value is interpreted as follows:
  ///    -1 = blocking operation
  ///     0 = non-blocking operation without retries
  ///    >0 = time (in milliseconds) after which the function returns an error,
  ///         if the operation was not accomplished
  Connection(const std::string& hostname, int port, int timeoutMs);

  /// Creates a new socket and connects it to an endpoint by a file socket.
  /// ZMQ IPC is used for the connection; the full address is thus
  ///     ipc://<file_socket>
  /// @param file_socket [in] file to use as the socket
  /// @param timeoutMs [in] Send / receive operation timeout in milliseconds
  ///     (default = -1), the value is interpreted as follows:
  ///    -1 = blocking operation
  ///     0 = non-blocking operation without retries
  ///    >0 = time (in milliseconds) after which the function returns an error,
  ///         if the operation was not accomplished
  Connection(const std::string& file_socket, int timeoutMs);

  /// Move constructor
  Connection(Connection&& conn);

  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  /// Send data in a string format over the socket connection
  /// @param data [in] Data to send to the socket
  /// @return true if the send operation succeeded
  bool send(const std::string& data);

  /// Send data in a general binary format over the socket connection
  /// @param buf [in] Buffer containing data to send to the socket
  /// @param len [in] Length of the data in buffer (in bytes)
  /// @return true if the send operation succeeded
  bool send(const void* buf, size_t len);

  /// Receive data over the socket connection and interpret it as a string
  /// @param dest [out] String to store the received data to
  /// @return true if the receive operation succeeded
  bool receive(std::string& dest);

  /// Receive data over the socket connection and interpret it as a vector
  /// @param dest [out] Vector to store the received data to
  /// @return true if the receive operation succeeded
  bool receive(std::vector<uint8_t>& dest);

  bool poll(long timeout);

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

} // namespace torchcraft
