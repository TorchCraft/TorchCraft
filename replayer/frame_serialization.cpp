/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
 
#include <algorithm>

#include "frame.h"

namespace torchcraft {
namespace replayer {    
  void writeTail(
      std::ostream& out,
      const std::unordered_map<int32_t, std::vector<replayer::Action>>& actions,
      const std::unordered_map<int32_t, replayer::Resources>& resources,
      const std::vector<replayer::Bullet>& bullets) {
    out << actions.size() << " ";
    for (auto& v : actions) {
      out << v.first << " " << v.second.size() << " ";
      for (auto& u : v.second) {
        out << u << " ";
      }
    }
    out << resources.size() << " ";
    for (auto& r : resources) {
      out << r.first << " " << r.second << " ";
    }
    out << bullets.size();
    for (auto& b : bullets) {
      out << " " << b;
    }
  }

  void readTail(
      std::istream& in,
      std::unordered_map<int32_t, std::vector<replayer::Action>>& actions,
      std::unordered_map<int32_t, replayer::Resources>& resources,
      std::vector<replayer::Bullet>& bullets) {
    int nPlayer, nBullets;

    in >> nPlayer;
    if (nPlayer < 0)
      throw std::runtime_error("Corrupted replay: actions nPlayer < 0");
    if (nPlayer > 9)
      throw std::runtime_error("Corrupted replay: actions nPlayer > 9");
    for (int32_t i = 0; i < nPlayer; i++) {
      int32_t idPlayer, nActions;
      in >> idPlayer >> nActions;
      if (nActions < 0)
        throw std::runtime_error("Corrupted replay: nActions < 0");
      if (nActions > 10000)
        throw std::runtime_error("Corrupted replay: nActions > 10000");
      actions[idPlayer] = std::vector<replayer::Action>();
      actions[idPlayer].resize(nActions);
      for (int32_t j = 0; j < nActions; j++) {
        in >> actions[idPlayer][j];
      }
    }

    in >> nPlayer;
    if (nPlayer < 0)
      throw std::runtime_error("Corrupted replay: resources nPlayer < 0");
    if (nPlayer > 9)
      throw std::runtime_error("Corrupted replay: resources nPlayer > 9");
    for (int32_t i = 0; i < nPlayer; i++) {
      int32_t idPlayer;
      in >> idPlayer;
      in >> resources[idPlayer];
    }

    in >> nBullets;
    if (nBullets < 0)
      throw std::runtime_error("Corrupted replay: nBullets < 0");
    if (nBullets > 10000)
      throw std::runtime_error("Corrupted replay: nBullets > 500");
    bullets.resize(nBullets);
    for (int32_t i = 0; i < nBullets; i++) {
      in >> bullets[i];
    }
  }
} // namespace torchcraft
} // namespace replayer