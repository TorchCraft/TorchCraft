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

#include <torchcraft/constants.h>

namespace torchcraft {

void init();

class State;
class Connection;

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

    Command() {}
    Command(int code) : code(code) {}
    Command(int code, std::string str) : code(code), str(std::move(str)) {}
    Command(int code, std::initializer_list<int>&& args)
        : code(code), args(args) {}
    Command(int code, std::string str, std::initializer_list<int>&& args)
        : code(code), args(args), str(std::move(str)) {}
    Command(int code, std::string str, std::vector<int>& args)
        : code(code), args(args), str(std::move(str)) {}
    template <typename... Args>
    Command(int code, int a, Args&&... args)
        : Command(code, {a, std::forward<Args>(args)...}) {}
    template <typename... Args>
    Command(int code, std::string str, Args&&... args)
        : Command(code, std::move(str), {std::forward<Args>(args)...}) {}
  };

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
  /// @param timeoutMs [in] Send / receive operation timeout in milliseconds,
  ///     the value is interpreted as follows:
  ///    -1 = blocking operation
  ///     0 = non-blocking operation without retries
  ///    >0 = time (in milliseconds) after which the function returns an error,
  ///         if the operation was not accomplished
  /// @return true if the connection was established; false otherwise
  bool connect(const std::string& hostname, int port, int timeoutMs);

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
  bool connect(const std::string& file_socket, int timeoutMs);

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
  /// @return true if the send operation succeeded, false otherwise
  bool send(const std::vector<Command>& commands);

  /// Receive a message containing state updates over the established socket
  /// connection
  /// @param updates [out] State field names that were updated from
  ///     the received message
  /// @return true if the receive operation succeeded, false otherwise
  bool receive(std::vector<std::string>& updates);

  /// Blocks until a message is available for receive().
  /// @return false on failure (timeout or lost connectivity), true otherwise
  bool poll(long timeout = -1);

  std::string error() const {
    return error_;
  }

  std::vector<Command> lastCommands() const {
    return lastCommands_;
  }
  std::vector<int8_t> lastCommandsStatus() const {
    return lastCommandsStatus_;
  }

  State* state() const {
    return state_;
  }

 private:
  bool connect(std::unique_ptr<Connection>&&);
  void clearError() {
    error_.clear();
  }

  // The connection is RAII and is created/reset in init().
  std::unique_ptr<Connection> conn_;
  State* state_;
  bool sent_;
  std::string error_;
  std::string uid_;
  std::vector<Command> lastCommands_;
  std::vector<int8_t> lastCommandsStatus_;
};

} // namespace torchcraft
