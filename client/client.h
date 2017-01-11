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

namespace client {

class Client {
 public:
  struct Options {
    std::string initialMap;
    int windowSize[2] = {-1, -1};
    int windowPos[2] = {-1, -1};
    bool microBattles = false;
  };

 public:
  explicit Client(Connection conn);

  bool init(std::string& dest, Options opts = Options());
  bool send(const std::string& msg);
  bool receive(std::string& dest);
  std::string error() const {
    return error_;
  }

 private:
  void clearError() {
    error_.clear();
  }

  Connection conn_;
  bool sent_;
  std::string error_;
};

} // namespace client
