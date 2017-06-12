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
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "refcount.h"

#ifdef _MSC_VER
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int int32_t;
#endif

// TODO Check types !

namespace torchcraft {
namespace replayer {
struct Order {
  int32_t first_frame; // first frame number where order appeared

  int32_t type; // see BWAPI::Orders::Enum
  int32_t targetId;
  int32_t targetX, targetY;

  bool operator==(const Order& o) const {
    // Ignore first_frame
    return type == o.type && targetId == o.targetId && targetX == o.targetX &&
        targetY == o.targetY;
  }
};

struct UnitCommand {
  int32_t frame;
  int32_t type; // see BWAPI::UnitCommandType::Enum
  int32_t targetId;
  int32_t targetX, targetY;
  int32_t extra;

  bool operator==(const UnitCommand& c) const {
    // Ignore frame
    return type == c.type && targetId == c.targetId && targetX == c.targetX &&
        targetY == c.targetY && extra == c.extra;
  }
};

struct Unit {
  int32_t id, x, y;
  int32_t health, max_health, shield, max_shield, energy;
  int32_t maxCD, groundCD, airCD;
  uint64_t flags;
  int32_t visible;
  int32_t type, armor, shieldArmor, size;

  int32_t pixel_x, pixel_y;
  int32_t pixel_size_x, pixel_size_y;

  int32_t groundATK, airATK;
  int32_t groundDmgType, airDmgType;
  int32_t groundRange, airRange;

  std::vector<Order> orders;
  UnitCommand command;

  double velocityX, velocityY;

  int32_t playerId;

  int32_t resources;

  int32_t buildType;
  int32_t techType;
  int32_t upgradeType;
  int32_t remainingBuildTrainTime;
  int32_t remainingUpgradeResearchTime;
  int32_t spellCD;
  int32_t associatedUnit; // addOn, nydusExit, transport, hatchery
  int32_t associatedCount; // spiderMines, scarabs, interceptors
  int32_t hasNuke;

  enum Flags : uint64_t {
    // clang-format off
    Accelerating       = 1ll << 0,
    Attacking          = 1ll << 1,
    AttackFrame        = 1ll << 2,
    BeingConstructed   = 1ll << 3,
    BeingGathered      = 1ll << 4,
    BeingHealed        = 1ll << 5,
    Blind              = 1ll << 6,
    Braking            = 1ll << 7,
    Burrowed           = 1ll << 8,
    CarryingGas        = 1ll << 9,
    CarryingMinerals   = 1ll << 10,
    Cloaked            = 1ll << 11,
    Completed          = 1ll << 12,
    Constructing       = 1ll << 13,
    DefenseMatrixed    = 1ll << 14,
    Detected           = 1ll << 15,
    Ensnared           = 1ll << 16,
    Flying             = 1ll << 17,
    Following          = 1ll << 18,
    GatheringGas       = 1ll << 19,
    GatheringMinerals  = 1ll << 20,
    Hallucination      = 1ll << 21,
    HoldingPosition    = 1ll << 22,
    Idle               = 1ll << 23,
    Interruptible      = 1ll << 24,
    Invincible         = 1ll << 25,
    Irradiated         = 1ll << 26,
    Lifted             = 1ll << 27,
    Loaded             = 1ll << 28,
    LockedDown         = 1ll << 29,
    Maelstrommed       = 1ll << 30,
    Morphing           = 1ll << 31,
    Moving             = 1ll << 32,
    Parasited          = 1ll << 33,
    Patrolling         = 1ll << 34,
    Plagued            = 1ll << 35,
    Powered            = 1ll << 36,
    Repairing          = 1ll << 37,
    Researching        = 1ll << 38,
    Selected           = 1ll << 39,
    Sieged             = 1ll << 40,
    StartingAttack     = 1ll << 41,
    Stasised           = 1ll << 42,
    Stimmed            = 1ll << 43,
    Stuck              = 1ll << 44,
    Targetable         = 1ll << 45,
    Training           = 1ll << 46,
    UnderAttack        = 1ll << 47,
    UnderDarkSwarm     = 1ll << 48,
    UnderDisruptionWeb = 1ll << 49,
    UnderStorm         = 1ll << 50,
    Upgrading          = 1ll << 51,
    // clang-format on
  };
};

std::ostream& operator<<(std::ostream& out, const Unit& o);
std::istream& operator>>(std::istream& in, Unit& o);

/*
    Terran_Infantry_Armor = 0,
    Terran_Vehicle_Plating = 1,
    Terran_Ship_Plating = 2,
    Terran_Infantry_Weapons = 7,
    Terran_Vehicle_Weapons = 8,
    Terran_Ship_Weapons = 9,
    Zerg_Carapace = 3,
    Zerg_Flyer_Carapace = 4,
    Protoss_Ground_Armor = 5,
    Protoss_Air_Armor = 6,
    Zerg_Melee_Attacks = 10,
    Zerg_Missile_Attacks = 11,
    Zerg_Flyer_Attacks = 12,
    Protoss_Ground_Weapons = 13,
    Protoss_Air_Weapons = 14,
    Protoss_Plasma_Shields = 15,
*/

struct Resources {
  int32_t ore;
  int32_t gas;
  int32_t used_psi;
  int32_t total_psi;
  uint64_t upgrades;
  uint64_t upgrades_level;
  uint64_t techs;

  // clang-format off
  enum Upgrades : uint64_t {
    Terran_Infantry_Armor   = 1ll << 0,
    Terran_Vehicle_Plating  = 1ll << 1,
    Terran_Ship_Plating     = 1ll << 2,
    Zerg_Carapace           = 1ll << 3,
    Zerg_Flyer_Carapace     = 1ll << 4,
    Protoss_Ground_Armor    = 1ll << 5,
    Protoss_Air_Armor       = 1ll << 6,
    Terran_Infantry_Weapons = 1ll << 7,
    Terran_Vehicle_Weapons  = 1ll << 8,
    Terran_Ship_Weapons     = 1ll << 9,
    Zerg_Melee_Attacks      = 1ll << 10,
    Zerg_Missile_Attacks    = 1ll << 11,
    Zerg_Flyer_Attacks      = 1ll << 12,
    Protoss_Ground_Weapons  = 1ll << 13,
    Protoss_Air_Weapons     = 1ll << 14,
    Protoss_Plasma_Shields  = 1ll << 15,
    U_238_Shells            = 1ll << 16,
    Ion_Thrusters           = 1ll << 17,
    Titan_Reactor           = 1ll << 19,
    Ocular_Implants         = 1ll << 20,
    Moebius_Reactor         = 1ll << 21,
    Apollo_Reactor          = 1ll << 22,
    Colossus_Reactor        = 1ll << 23,
    Ventral_Sacs            = 1ll << 24,
    Antennae                = 1ll << 25,
    Pneumatized_Carapace    = 1ll << 26,
    Metabolic_Boost         = 1ll << 27,
    Adrenal_Glands          = 1ll << 28,
    Muscular_Augments       = 1ll << 29,
    Grooved_Spines          = 1ll << 30,
    Gamete_Meiosis          = 1ll << 31,
    Metasynaptic_Node       = 1ll << 32,
    Singularity_Charge      = 1ll << 33,
    Leg_Enhancements        = 1ll << 34,
    Scarab_Damage           = 1ll << 35,
    Reaver_Capacity         = 1ll << 36,
    Gravitic_Drive          = 1ll << 37,
    Sensor_Array            = 1ll << 38,
    Gravitic_Boosters       = 1ll << 39,
    Khaydarin_Amulet        = 1ll << 40,
    Apial_Sensors           = 1ll << 41,
    Gravitic_Thrusters      = 1ll << 42,
    Carrier_Capacity        = 1ll << 43,
    Khaydarin_Core          = 1ll << 44,
    Argus_Jewel             = 1ll << 47,
    Argus_Talisman          = 1ll << 49,
    Caduceus_Reactor        = 1ll << 51,
    Chitinous_Plating       = 1ll << 52,
    Anabolic_Synthesis      = 1ll << 53,
    Charon_Boosters         = 1ll << 54,
    Upgrade_60              = 1ll << 60,
    Unknow                  = 1ll << 62,
  };
  enum UpgradesLevel : uint64_t { // could be uint32_t
    Terran_Infantry_Armor_2   = 1ll << 0,
    Terran_Vehicle_Plating_2  = 1ll << 1,
    Terran_Ship_Plating_2     = 1ll << 2,
    Terran_Infantry_Weapons_2 = 1ll << 7,
    Terran_Vehicle_Weapons_2  = 1ll << 8,
    Terran_Ship_Weapons_2     = 1ll << 9,
    Zerg_Carapace_2           = 1ll << 3,
    Zerg_Flyer_Carapace_2     = 1ll << 4,
    Protoss_Ground_Armor_2    = 1ll << 5,
    Protoss_Air_Armor_2       = 1ll << 6,
    Zerg_Melee_Attacks_2      = 1ll << 10,
    Zerg_Missile_Attacks_2    = 1ll << 11,
    Zerg_Flyer_Attacks_2      = 1ll << 12,
    Protoss_Ground_Weapons_2  = 1ll << 13,
    Protoss_Air_Weapons_2     = 1ll << 14,
    Protoss_Plasma_Shields_2  = 1ll << 15,
    Terran_Infantry_Armor_3   = 1ll << 16,
    Terran_Vehicle_Plating_3  = 1ll << 17,
    Terran_Ship_Plating_3     = 1ll << 18,
    Terran_Infantry_Weapons_3 = 1ll << 23,
    Terran_Vehicle_Weapons_3  = 1ll << 24,
    Terran_Ship_Weapons_3     = 1ll << 25,
    Zerg_Carapace_3           = 1ll << 19,
    Zerg_Flyer_Carapace_3     = 1ll << 20,
    Protoss_Ground_Armor_3    = 1ll << 21,
    Protoss_Air_Armor_3       = 1ll << 22,
    Zerg_Melee_Attacks_3      = 1ll << 26,
    Zerg_Missile_Attacks_3    = 1ll << 27,
    Zerg_Flyer_Attacks_3      = 1ll << 28,
    Protoss_Ground_Weapons_3  = 1ll << 29,
    Protoss_Air_Weapons_3     = 1ll << 30,
    Protoss_Plasma_Shields_3  = 1ll << 31,
  };
  enum Techs : uint64_t {
    Stim_Packs         = 1ll << 0,
    Lockdown           = 1ll << 1,
    EMP_Shockwave      = 1ll << 2,
    Spider_Mines       = 1ll << 3,
    Scanner_Sweep      = 1ll << 4,
    Tank_Siege_Mode    = 1ll << 5,
    Defensive_Matrix   = 1ll << 6,
    Irradiate          = 1ll << 7,
    Yamato_Gun         = 1ll << 8,
    Cloaking_Field     = 1ll << 9,
    Personnel_Cloaking = 1ll << 10,
    Burrowing          = 1ll << 11,
    Infestation        = 1ll << 12,
    Spawn_Broodlings   = 1ll << 13,
    Dark_Swarm         = 1ll << 14,
    Plague             = 1ll << 15,
    Consume            = 1ll << 16,
    Ensnare            = 1ll << 17,
    Parasite           = 1ll << 18,
    Psionic_Storm      = 1ll << 19,
    Hallucination      = 1ll << 20,
    Recall             = 1ll << 21,
    Stasis_Field       = 1ll << 22,
    Archon_Warp        = 1ll << 23,
    Restoration        = 1ll << 24,
    Disruption_Web     = 1ll << 25,
    Unused_26          = 1ll << 26,
    Mind_Control       = 1ll << 27,
    Dark_Archon_Meld   = 1ll << 28,
    Feedback           = 1ll << 29,
    Optical_Flare      = 1ll << 30,
    Maelstrom          = 1ll << 31,
    Lurker_Aspect      = 1ll << 32,
    Unused_33          = 1ll << 33,
    Healing            = 1ll << 34,
    Nuclear_Strike     = 1ll << 45,
    Unknown            = 1ll << 46,
  };
  // clang-format on
};

std::ostream& operator<<(std::ostream& out, const Resources& r);
std::istream& operator>>(std::istream& in, Resources& r);

struct Bullet {
  int32_t type, x, y;
};

std::ostream& operator<<(std::ostream& out, const Bullet& o);
std::istream& operator>>(std::istream& in, Bullet& o);

struct Action { // corresponds to a torchcraft message
  std::vector<int32_t> action;
  int32_t uid;
  int32_t aid;
};

std::ostream& operator<<(std::ostream& out, const Action& o);
std::istream& operator>>(std::istream& in, Action& o);

class Frame : public RefCounted {
 public:
  // The keys of these hash tables are the players' ids.
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

  Frame(const Frame& o)
      : RefCounted(),
        units(o.units),
        actions(o.actions),
        resources(o.resources),
        bullets(o.bullets) {
    reward = o.reward;
    is_terminal = o.is_terminal;
  }

  Frame(const Frame* o)
      : RefCounted(),
        units(o->units),
        actions(o->actions),
        resources(o->resources),
        bullets(o->bullets) {
    reward = o->reward;
    is_terminal = o->is_terminal;
  }

  Frame(Frame&& o)
      : RefCounted() {
    swap(*this, o);
  }

  friend void swap(Frame& a, Frame& b) {
    using std::swap;
    swap(a.units, b.units);
    swap(a.actions, b.actions);
    swap(a.resources, b.resources);
    swap(a.bullets, b.bullets);
    swap(a.reward, b.reward);
    swap(a.is_terminal, b.is_terminal);
  }

  Frame& operator=(Frame other) {
    swap(*this, other);
    return *this;
  }

  void clear() {
    units.clear();
    actions.clear();
    resources.clear();
    bullets.clear();
    reward = 0;
    is_terminal = 0;
  }

  void filter(int32_t x, int32_t y, Frame& o) const {
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
    reward = next_frame.reward;
    is_terminal = next_frame.is_terminal;
  }
}; // class Frame
std::ostream& operator<<(std::ostream& out, const Frame& o);
std::istream& operator>>(std::istream& in, Frame& o);

// Frame diffs
class FrameDiff;

namespace detail {
class UnitDiff {
 public:
  int id;
  std::vector<int32_t> var_ids;
  std::vector<int32_t> var_diffs;
  std::vector<int32_t> order_ids;
  std::vector<int32_t> order_diffs;
  int32_t order_size;
  double velocityX, velocityY;
  int64_t flags;
};

Frame* add(Frame* frame, FrameDiff* diff);

inline bool orderUnitByiD(const Unit& a, const Unit& b) {
  return (a.id < b.id);
}

bool frameEq(Frame* f1, Frame* f2, bool debug = true);

} // namespace detail

class FrameDiff {
 public:
  std::vector<int32_t> pids;
  std::vector<std::vector<detail::UnitDiff>> units;
  // These are unlikely to be the same, so we just copy.
  std::unordered_map<int32_t, std::vector<Action>> actions;
  std::unordered_map<int32_t, Resources> resources;
  std::vector<Bullet> bullets;
  int reward;
  int is_terminal;
};

// These diffing functions will order the IDs of units in each frame, and thus
// is not const.
FrameDiff frame_diff(Frame& lhs, Frame& rhs);
FrameDiff frame_diff(Frame* lhs, Frame* rhs);
Frame* frame_undiff(FrameDiff& lhs, Frame& rhs);
Frame* frame_undiff(Frame& lhs, FrameDiff& rhs);
Frame* frame_undiff(FrameDiff* lhs, Frame* rhs);
Frame* frame_undiff(Frame* lhs, FrameDiff* rhs);

std::ostream& operator<<(std::ostream& out, const FrameDiff& o);
std::ostream& operator<<(std::ostream& out, const detail::UnitDiff& o);
std::istream& operator>>(std::istream& in, FrameDiff& o);
std::istream& operator>>(std::istream& in, detail::UnitDiff& o);
} // namespace replayer
} // namespace torchcraft
