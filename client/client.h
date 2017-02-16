/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <memory>
#include <set>
#include <vector>

#include "connection.h"
#include "constants.h"

extern "C" {
#include <lua.h>
}

namespace client {

void init();

class State;

class Client {
 public:
  struct Options {
    std::string initial_map;
    int window_size[2];
    int window_pos[2];
    bool micro_battles;

    // A subset of unit types to consider when checking for end-of-game
    // condition, for example.
    std::set<BW::UnitType> only_consider_types;

    Options() : window_size{-1, -1}, window_pos{-1, -1}, micro_battles(false) {}
  };

  struct Command {
    int code = -1;
    std::vector<int> args;
    std::string str;
  };

 public:

  // LIFECYCLE

  Client();
  ~Client();
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  // OPERATIONS

  /// Create a socket connection and connect it to an endpoint specified by
  /// a TCP address parametrized by a hostname and a port. The final endpoint
  /// is defined as tcp://<hostname>:<port>
  /// @param hostname [in] Hostname part of a TCP address for socket connection
  /// @param port [in] Port part of a TCP address for socket connection
  /// @param send_timeout_ms [in] Send operation timeout in milliseconds
  ///     (default = -1), the value is interpreted as follows:
  ///    -1 = blocking send operation
  ///     0 = non-blocking send operation without retries
  ///    >0 = time (in milliseconds) after which the function returns an error,
  ///         if the send operation was not accomplished
  /// @param receive_timeout_ms [in] Receive operation timeout in milliseconds
  ///     (default = -1), the value is interpreted as follows:
  ///    -1 = blocking receive operation
  ///     0 = non-blocking receive operation without retries
  ///    >0 = time (in milliseconds) after which the function returns an error,
  ///         if the receive operation was not accomplished
  /// @return true if the connection was established; false otherwise
  bool connect(const std::string& hostname, int port,
      int send_timeout_ms = -1, int receive_timeout_ms = -1);

  /// Indicates whether the connection was successfully established
  /// @return true if the connection was successfully established;
  ///     false otherwise
  bool connected() const {
    return conn_ != nullptr;
  }

  /// Close the socket connection if it was previously successfully established
  /// and destroy the associated socket
  /// @return true if the connection was closed successfully; false if the
  ///     connection did not exist
  bool close();

  /// Perform handshake over the established connection.
  /// @param updates [out] State field names that were updated from
  ///     the handshake response
  /// @param opts [in] Options to pass in the handshake message
  ///     (default = Options())
  /// @return true if the handshake succeeded; false otherwise
  bool init(std::vector<std::string>& updates, const Options& opts = Options());

  /// Send a message containing commands over the established socket connection
  /// @param commands [in] Commands to send over the socket connection
  /// @return false if one of the following conditions holds:
  ///     1) this is a second send operation in a row (without an
  ///        intermediate receive operation);
  ///     2) the socket connection has not been successfully established
  ///     3) send operation failed due to an error or a timeout
  ///     Otherwise, returns true
  bool send(const std::vector<Command>& commands);

  /// Receive a message containing state updates over the established socket
  /// connection
  /// @param updates [out] State field names that were updated from
  ///     the received message
  /// @return false if one of the following conditions holds:
  ///     1) this is a second receive operation in a row (without an
  ///        intermediate send operation);
  ///     2) the socket connection has not been successfully established
  ///     3) receive operation failed due to an error or a timeout
  ///     4) unknown message type was encountered
  ///     Otherwise, returns true
  bool receive(std::vector<std::string>& updates);

  std::string error() const {
    return error_;
  }

  State* state() const {
    return state_;
  }

 private:
  void clearError() {
    error_.clear();
  }

  // The connection is RAII and is created/reset in init().
  std::unique_ptr<Connection> conn_;
  State* state_;
  bool sent_;
  std::string error_;
};

} // namespace client
