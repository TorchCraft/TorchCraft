/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "connection.h"
#include "state.h"

extern "C" {
#include <lua.h>
}

namespace client {

void init();

class Client {
 public:
  struct Options {
    std::string initial_map;
    int window_size[2] = {-1, -1};
    int window_pos[2] = {-1, -1};
    bool micro_battles = false;
  };

  struct Command {
    int code = -1;
    std::vector<int> args;
    std::string str;
  };

 public:
  Client();
  ~Client();

  bool connect(const std::string& hostname, int port);
  bool connected() const {
    return conn_ != nullptr;
  }
  bool close();
  bool init(std::vector<std::string>& updates, const Options& opts = Options());
  bool send(const std::vector<Command>& commands);
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

  std::unique_ptr<Connection> conn_;
  State* state_;
  bool sent_;
  std::string error_;
};

} // namespace client
