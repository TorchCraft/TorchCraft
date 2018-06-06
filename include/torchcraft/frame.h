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

#include <torchcraft/refcount.h>
#include "messages_generated.h"

#ifdef _MSC_VER
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int int32_t;
#endif

namespace torchcraft {
namespace replayer {

struct Unit;
struct Resources;
struct Bullet;
struct Action;
class Frame;
class FrameDiff;
namespace detail { class UnitDiff; }

std::ostream& operator<<(std::ostream& out, const Frame& o);
std::istream& operator>>(std::istream& in, Frame& o);
std::ostream& operator<<(std::ostream& out, const FrameDiff& o);
std::istream& operator>>(std::istream& in, FrameDiff& o);

struct Bullet {
  int32_t type, x, y;
};

struct Action {
  std::vector<int32_t> action;
  int32_t uid;
  int32_t aid;
};

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
  int32_t buildTechUpgradeType;
  int32_t remainingBuildTrainTime;
  int32_t remainingUpgradeResearchTime;
  int32_t spellCD;
  int32_t associatedUnit; // addOn, nydusExit, transport, hatchery
  int32_t associatedCount; // spiderMines, scarabs, interceptors, nuke

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

class Frame : public RefCounted {
 public:
  // The keys of these hash tables are the players' ids.
  std::unordered_map<int32_t, std::vector<Unit>> units;
  std::unordered_map<int32_t, std::vector<Action>> actions;
  std::unordered_map<int32_t, Resources> resources;
  std::vector<Bullet> bullets;
  std::vector<uint8_t> creep_map; // Do not access directly
  uint32_t width, height;
  int reward;
  int is_terminal;

  Frame();
  Frame(Frame&& o);
  Frame(const Frame& o);
  Frame(const Frame* o);
  Frame& operator=(Frame other) noexcept;

  void swap(Frame& a, Frame& b);
  void clear();
  void filter(int32_t x, int32_t y, Frame& o) const;
  void combine(const Frame& next_frame);
  bool getCreepAt(uint32_t x, uint32_t y);
  
  flatbuffers::Offset<fbs::Frame> addToFlatBufferBuilder(flatbuffers::FlatBufferBuilder& builder) const;
  void readFromFlatBufferTable(const fbs::Frame& table);
}; // class Frame

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
  void add(Frame* res, Frame* frame, FrameDiff* diff);

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
  std::unordered_map<uint32_t, uint32_t> creep_map;
  // Width and height never changes, so we don't diff them
  int reward;
  int is_terminal;
  
  flatbuffers::Offset<fbs::FrameDiff> addToFlatBufferBuilder(flatbuffers::FlatBufferBuilder& builder) const;
  void readFromFlatBufferTable(const fbs::FrameDiff& fbsFrameDiff);
};

void writeTail(
    std::ostream& out,
    const std::unordered_map<int32_t, std::vector<replayer::Action>>& actions,
    const std::unordered_map<int32_t, replayer::Resources>& resources,
    const std::vector<replayer::Bullet>& bullets);

void readTail(
    std::istream& in,
    std::unordered_map<int32_t, std::vector<replayer::Action>>& actions,
    std::unordered_map<int32_t, replayer::Resources>& resources,
    std::vector<replayer::Bullet>& bullets);

// These diffing functions will order the IDs of units in each frame, and thus
// is not const.
FrameDiff frame_diff(Frame&, Frame&);
FrameDiff frame_diff(Frame*, Frame*);
Frame* frame_undiff(FrameDiff*, Frame*);
Frame* frame_undiff(Frame*, FrameDiff*);
void frame_undiff(Frame* result, FrameDiff*, Frame*);
void frame_undiff(Frame* result, Frame*, FrameDiff*);

} // namespace replayer
} // namespace torchcraft
