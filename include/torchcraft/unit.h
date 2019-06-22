/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <torchcraft/primitives.h>
#include <vector>

namespace torchcraft {

struct State;

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

  State* state;

  bool isMine() const;
  bool isNeutral() const;
  bool isEnemy() const {
    return ! isMine() && ! isNeutral();
  }
  bool flag(Flags n) const {
    return (flags & n) != 0;
  }
  bool attacking() const {
    return flag(Flags::Attacking);
  }
  bool burrowed() const {
    return flag(Flags::Burrowed);
  }
  bool cloaked() const {
    return flag(Flags::Cloaked);
  }
  bool idle() const {
    return flag(Flags::Idle);
  }
  bool completed() const {
    return flag(Flags::Completed);
  }
  bool detected() const {
    return flag(Flags::Detected);
  }
  bool morphing() const {
    return flag(Flags::Morphing);
  }
  bool beingGathered() const {
    return flag(Flags::BeingGathered);
  }
  bool active() const {
    auto constexpr flip = Flags::Powered | Flags::Completed;
    auto constexpr mask = Flags::BeingConstructed |
        Flags::Completed | Flags::Loaded |
        Flags::LockedDown | Flags::Maelstrommed |
        Flags::Powered | Flags::Stasised |
        Flags::Stuck;
    return ((flags ^ flip) & mask) == 0;
  }
  bool powered() const {
    return flag(Flags::Powered);
  }
  bool lifted() const {
    return flag(Flags::Lifted);
  }
  bool carryingMinerals() const {
    return flag(Flags::CarryingMinerals);
  }
  bool carryingGas() const {
    return flag(Flags::CarryingGas);
  }
  bool carryingResources() const {
    return carryingMinerals() || carryingGas();
  }
  bool moving() const {
    return flag(Flags::Moving);
  }
  bool upgrading() const {
    return flag(Flags::Upgrading);
  }
  bool researching() const {
    return flag(Flags::Researching);
  }
  bool blind() const {
    return flag(Flags::Blind);
  }
  bool beingConstructed() const {
    return flag(Flags::BeingConstructed);
  }
  bool flying() const {
    return flag(Flags::Flying);
  }
  bool invincible() const {
    return flag(Flags::Invincible);
  }
  bool irradiated() const {
    return flag(Flags::Irradiated);
  }
  bool plagued() const {
    return flag(Flags::Plagued);
  }
  bool underDarkSwarm() const {
    return flag(Flags::UnderDarkSwarm);
  }
  bool gatheringGas() const {
    return flag(Flags::GatheringGas);
  }
  bool gatheringMinerals() const {
    return flag(Flags::GatheringMinerals);
  }
  bool gathering() const {
    return gatheringGas() || gatheringMinerals();
  }
  bool constructing() const {
    return flag(Flags::Constructing);
  }
  bool repairing() const {
    return flag(Flags::Repairing);
  }
  bool stimmed() const {
    return flag(Flags::Stimmed);
  }
  bool ensnared() const {
    return flag(Flags::Ensnared);
  }
};

} //namespace replayer
} //namespace torchcraft