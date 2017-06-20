/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <set>
#include <vector>

#include "constants.h"
#include "frame.h"
#include "refcount.h"

// Flatbuffer messages
namespace torchcraft {
namespace fbs {
struct HandshakeServer;
struct Frame;
struct EndGame;
struct FrameData;
}
}

namespace torchcraft {

// Aliases for basic replayer types provided for convenience
typedef replayer::Unit Unit;
typedef replayer::Order Order;
typedef replayer::Resources Resources;
typedef replayer::Bullet Bullet;
typedef replayer::Action Action;
typedef replayer::Frame Frame;

class State : public RefCounted {
 public:
  struct Position {
    int x, y;
    Position() : x(0), y(0) {}
    Position(int x, int y) : x(x), y(y) {}
  };

  // setup
  int lag_frames; // number of frames from order to execution
  int map_size[2];  // build tile resolution
  std::vector<uint8_t> ground_height_data; // 2D. build tile resolution
  std::vector<uint8_t> walkable_data; // 2D, walk tile resolution (build x 4)
  std::vector<uint8_t> buildable_data; // 2D, build tile resolution
  std::string map_name; // Name on the current map
  std::vector<Position> start_locations;
  int player_id;
  int neutral_id;
  bool replay;

  // game state
  Frame* frame; // this will allow for easy reset (XXX)
  std::string frame_string;
  std::vector<int> deaths;
  int frame_from_bwapi;
  int battle_frame_count; // if micro mode

  bool game_ended; // did the game end?
  bool game_won;

  // if micro mode
  bool battle_just_ended;
  bool battle_won;
  bool waiting_for_restart;
  int last_battle_ended;

  // if with image
  std::string img_mode;
  int screen_position[2]; // position of screen {x, y} in pixels. {0, 0} is
  // top-left
  std::vector<uint8_t> visibility;
  int visibility_size[2];
  std::vector<uint8_t> image; // RGB
  int image_size[2];

  // Alive units in this frame. Used to detect end-of-battle in micro mode. If
  // the current frame is the end of a battle, this will contain all units that
  // were alive when the battle ended (which is not necessarily the current
  // frame due to frame skipping on the serv side). Note that this map ignores
  // onlyConsiderUnits_.
  // Maps unit id to player id
  std::unordered_map<int32_t, int32_t> aliveUnits;

  // Like aliveUnits, but containing only units of types in onlyConsiderUnits.
  // If onlyConsiderUnits is empty, this map is invalid.
  std::unordered_map<int32_t, int32_t> aliveUnitsConsidered;

  // Bots might want to use this map instead of frame->units because:
  // - Unknown unit types are not present (e.g. map revealers)
  // - Units reported as dead are not present (important if the server performs
  //   frame skipping. In that case, frame->units will still contain all units
  //   that have died since the last update.
  // - In micro mode and with frame skipping, deaths are only applied until the
  //   battle is considered finished, i.e. it corresponds to aliveUnits.
  std::unordered_map<int32_t, std::vector<Unit>> units;

  // Total number of updates received since creation (resets are counted as
  // well).
  uint64_t numUpdates = 0;

  explicit State(
      bool microBattles = false,
      std::set<BW::UnitType> onlyConsiderTypes = std::set<BW::UnitType>());
  State(const State& other);
  State(State&& other);
  ~State();
  State& operator=(State other);
  friend void swap(State& a, State& b);

  bool microBattles() const {
    return microBattles_;
  }
  const std::set<BW::UnitType>& onlyConsiderTypes() const {
    return onlyConsiderTypes_;
  }
  void setMicroBattles(bool microBattles) {
    microBattles_ = microBattles;
  }
  void setOnlyConsiderTypes(std::set<BW::UnitType> types) {
    onlyConsiderTypes_ = std::move(types);
    aliveUnitsConsidered.clear();
  }

  void reset();
  std::vector<std::string> update(
      const torchcraft::fbs::HandshakeServer* handshake);
  std::vector<std::string> update(const torchcraft::fbs::Frame* frame);
  std::vector<std::string> update(const torchcraft::fbs::EndGame* end);
  void trackAliveUnits(
      std::vector<std::string>& upd,
      const std::set<BW::UnitType>& considered);

  int getUpgradeLevel(BW::UpgradeType ut) {
    if (!(frame->resources[player_id].upgrades & (1ll << ut))) return 0;
    const auto NB_LVLABLE_UPGRADES = 16;
    if (ut >= NB_LVLABLE_UPGRADES) return 1;
    uint64_t lvls = frame->resources[player_id].upgrades_level;
    if (lvls & (1ll << ut)) return 2;
    if (lvls & (1ll << (ut + NB_LVLABLE_UPGRADES))) return 3;
    return 1;
  }

  bool hasResearched(BW::TechType tt) {
    if (frame->resources[player_id].techs & (1ll << tt)) return true;
    return false;
  }

 private:
  bool setRawImage(const torchcraft::fbs::Frame* frame);
  void postUpdate(std::vector<std::string>& upd);
  bool checkBattleFinished(std::vector<std::string>& upd);
  bool update_frame(const torchcraft::fbs::FrameData* fd);

  bool microBattles_;
  std::set<BW::UnitType> onlyConsiderTypes_;
};

} // namespace torchcraft
