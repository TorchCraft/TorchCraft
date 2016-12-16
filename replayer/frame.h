/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <cassert>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iostream>

#include "refcount.h"

#ifdef _MSC_VER
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int int32_t;
#endif

//TODO Check types !

namespace replayer {
  struct Order {
    int32_t first_frame; // first frame number where order appeared

    int32_t type;       // see BWAPI::Orders::Enum
    int32_t targetId;
    int32_t targetX, targetY;

    bool operator==(const Order& o) const {
      return type == o.type && targetId == o.targetId
        && targetX == o.targetX && targetY == o.targetY;
    }
  };

  struct Unit {
    int32_t id, x, y;
    int32_t health, max_health, shield, max_shield, energy;
    int32_t maxCD, groundCD, airCD;
    bool idle, visible;
    int32_t type, armor, shieldArmor, size;

    int32_t pixel_x, pixel_y;
    int32_t pixel_size_x, pixel_size_y;

    int32_t groundATK, airATK;
    int32_t groundDmgType, airDmgType;
    int32_t groundRange, airRange;

    std::vector<Order> orders;

    double velocityX, velocityY;

    int32_t playerId;

    int32_t resources;
  };

  std::ostream& operator<<(std::ostream& out, const Unit& o);
  std::istream& operator>>(std::istream& in, Unit& o);

  struct Resources {
    int32_t ore;
    int32_t gas;
    int32_t used_psi;
    int32_t total_psi;
  };

  std::ostream& operator<<(std::ostream& out, const Resources& r);
  std::istream& operator>>(std::istream& in, Resources& r);

  struct Bullet {
    int32_t type, x, y;
  };

  std::ostream& operator<<(std::ostream& out, const Bullet& o);
  std::istream& operator>>(std::istream& in, Bullet& o);

  struct Action {         // corresponds to a torchcraft message
    std::vector<int32_t> action;
    int32_t uid;
    int32_t aid;
  };

  class Frame : public RefCounted {
  public:
    //The keys of these hash tables are the players' ids.
    std::unordered_map<int32_t, std::vector<Unit>> units;
    std::unordered_map<int32_t, std::vector<Action>> actions;
    std::unordered_map<int32_t, Resources> resources;
    std::vector<Bullet> bullets;
    int reward;
    int is_terminal;

    Frame() : RefCounted() {
      reward = 0;
      is_terminal = 0;
    }
    Frame(const Frame& o) : RefCounted(),
      units(o.units), actions(o.actions),
      resources(o.resources), bullets(o.bullets)
    {
      reward = o.reward;
      is_terminal = o.is_terminal;
    }

    void filter(int32_t x, int32_t y, Frame& o) const {
      auto inRadius = [x, y](int32_t ux, int32_t uy) {
        return (x / 8 - ux)*(x / 8 - ux) + (y / 8 - uy)*(y / 8 - uy) <= 20 * 4 * 20 * 4;
      };

      for (auto& player : units) {
        o.units[player.first] = std::vector<Unit>();
        for (auto& unit : player.second) {
          if (inRadius(unit.x, unit.y)){
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

    void combine(const Frame& next_frame) {
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
          }
          else {
            int32_t i = idx[unit.id];
            // Take unit state from next frame but accumulate orders
            // so as to have a vector of all the orders taken
            std::vector<Order> ords
              = std::move(units[player_id][i].orders);
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
        if (next_frame.resources.find(player_id) != next_frame.resources.end())
        {
          auto next_res = next_frame.resources.at(player_id);
          resources[player_id].ore = next_res.ore;
          resources[player_id].gas = next_res.gas;
          resources[player_id].used_psi = next_res.used_psi;
          resources[player_id].total_psi = next_res.total_psi;
        }
      }
      // For other stuff, simply keep that of next_frame
      actions = next_frame.actions;
      bullets = next_frame.bullets;
      reward = next_frame.reward;
      is_terminal = next_frame.is_terminal;
    }
  };
  std::ostream& operator<<(std::ostream& out, const Frame& o);
  std::istream& operator>>(std::istream& in, Frame& o);
} // replayer
