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

  Frame::Frame() : RefCounted() {
    reward = 0;
    is_terminal = 0;
  }

  Frame::Frame(Frame&& o) : RefCounted() {
    swap(*this, o);
  }

  Frame::Frame(const Frame& o)
      : RefCounted(),
        units(o.units),
        actions(o.actions),
        resources(o.resources),
        bullets(o.bullets),
        creep_map(o.creep_map),
        width(o.width),
        height(o.height) {
    reward = o.reward;
    is_terminal = o.is_terminal;
  }

  Frame::Frame(const Frame* o)
      : RefCounted(),
        units(o->units),
        actions(o->actions),
        resources(o->resources),
        bullets(o->bullets),
        creep_map(o->creep_map),
        width(o->width),
        height(o->height) {
    reward = o->reward;
    is_terminal = o->is_terminal;
  }


  void Frame::swap(Frame& a, Frame& b) {
    using std::swap;
    swap(a.units, b.units);
    swap(a.actions, b.actions);
    swap(a.resources, b.resources);
    swap(a.bullets, b.bullets);
    swap(a.creep_map, b.creep_map);
    swap(a.width, b.width);
    swap(a.height, b.height);
    swap(a.reward, b.reward);
    swap(a.is_terminal, b.is_terminal);
  }

  Frame& Frame::operator=(Frame& other) {
    swap(*this, other);
    return *this;
  }

  void Frame::clear() {
    units.clear();
    actions.clear();
    resources.clear();
    bullets.clear();
    creep_map.clear();
    width = 0;
    height = 0;
    reward = 0;
    is_terminal = 0;
  }

  void Frame::filter(int32_t x, int32_t y, Frame& o) const {
    auto inRadius = [x, y](int32_t ux, int32_t uy) {
      return (x / 8 - ux) * (x / 8 - ux) + (y / 8 - uy) * (y / 8 - uy) <=
          20 * 4 * 20 * 4;
    };

    for (auto& player : units) {
      o.units[player.first] = std::vector<Unit>();
      for (auto& unit : player.second) {
        if (inRadius(unit.x, unit.y)) {
          o.units[player.first].push_back(unit);
        }
      }
    }
    for (auto& bullet : bullets) {
      if (inRadius(bullet.x, bullet.y)) {
        o.bullets.push_back(bullet);
      }
    }
  }

  void Frame::combine(const Frame& next_frame) {
    // For units, accumulate presence and commands
    for (auto& player : next_frame.units) {
      auto& player_id = player.first;
      auto& player_units = player.second;

      if (units.count(player_id) == 0) {
        units.insert(player);
        continue;
      }

      // Build dictionary of uid -> position in current frame unit vector
      std::unordered_map<int32_t, int32_t> idx;
      for (unsigned i = 0; i < units[player_id].size(); i++) {
        idx[units[player_id][i].id] = i;
      }
      // Iterate over units in next frame
      for (auto& unit : player_units) {
        if (idx.count(unit.id) == 0) {
          // Unit wasn't in current frame, add it
          units[player_id].push_back(unit);
        } else {
          int32_t i = idx[unit.id];
          // Take unit state from next frame but accumulate orders
          // so as to have a vector of all the orders taken
          std::vector<Order> ords = std::move(units[player_id][i].orders);
          ords.reserve(ords.size() + unit.orders.size());
          for (auto& ord : unit.orders) {
            if (ords.empty() || !(ord == ords.back())) {
              ords.push_back(ord);
            }
          }
          units[player_id][i] = unit;
          units[player_id][i].orders = std::move(ords);
        }
      }
      // For resources: keep the ones of the next frame
      if (next_frame.resources.find(player_id) != next_frame.resources.end()) {
        auto next_res = next_frame.resources.at(player_id);
        resources[player_id].ore = next_res.ore;
        resources[player_id].gas = next_res.gas;
        resources[player_id].used_psi = next_res.used_psi;
        resources[player_id].total_psi = next_res.total_psi;
        resources[player_id].upgrades = next_res.upgrades;
        resources[player_id].upgrades_level = next_res.upgrades_level;
        resources[player_id].techs = next_res.techs;
      }
    }
    // For other stuff, simply keep that of next_frame
    actions = next_frame.actions;
    bullets = next_frame.bullets;
    creep_map = next_frame.creep_map;
    width = next_frame.width;
    height = next_frame.height;
    reward = next_frame.reward;
    is_terminal = next_frame.is_terminal;
  }

  bool Frame::getCreepAt(uint32_t x, uint32_t y) {
    auto ind = (y / 4) * (this->width / 4) + (x / 4); // Convert to buildtiles
    return (this->creep_map[ind / 8] >> (ind % 8)) & 1;
  }
} // namespace replayer
} // namespace torchcraft