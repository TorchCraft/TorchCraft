/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "state.h"

#include "BWEnv/fbs/messages_generated.h"

namespace client {

State::State(bool microBattles, std::set<BW::UnitType> onlyConsiderTypes)
    : RefCounted(),
      frame(new replayer::Frame()),
      microBattles_(microBattles),
      onlyConsiderTypes_(std::move(onlyConsiderTypes)) {
  reset();
}

State::~State() {
  frame->decref();
}

void State::reset() {
  replay = false;
  lag_frames = 0;
  map_data.clear();
  map_data_size[0] = 0;
  map_data_size[1] = 0;
  map_name.clear();
  frame_string.clear();
  frame->clear();
  deaths.clear();
  frame_from_bwapi = 0;
  battle_frame_count = 0;
  game_ended = false;
  game_won = false;
  battle_just_ended = false;
  battle_won = false;
  waiting_for_restart = microBattles_;
  last_battle_ended = 0;
  img_mode.clear();
  screen_position[0] = -1;
  screen_position[1] = -1;
  image.clear(); // XXX invalidates existing tensors pointing to it
  image_size[0] = 0;
  image_size[1] = 0;
  aliveUnits.clear();
  aliveUnitsConsidered.clear();
  units.clear();
}

std::vector<std::string> State::update(
    const TorchCraft::HandshakeServer* handshake) {
  reset();

  std::vector<std::string> upd;
  lag_frames = handshake->lag_frames();
  upd.emplace_back("lag_frames");
  if (flatbuffers::IsFieldPresent(
          handshake, TorchCraft::HandshakeServer::VT_MAP_DATA)) {
    map_data.assign(
        handshake->map_data()->begin(), handshake->map_data()->end());
    upd.emplace_back("map_data");
  }
  if (flatbuffers::IsFieldPresent(
          handshake, TorchCraft::HandshakeServer::VT_MAP_SIZE)) {
    map_data_size[0] = handshake->map_size()->x();
    map_data_size[1] = handshake->map_size()->y();
  }
  if (flatbuffers::IsFieldPresent(
          handshake, TorchCraft::HandshakeServer::VT_MAP_NAME)) {
    map_name = handshake->map_name()->str();
    upd.emplace_back("map_name");
  }
  player_id = handshake->player_id();
  upd.emplace_back("player_id");
  neutral_id = handshake->neutral_id();
  upd.emplace_back("neutral_id");
  battle_frame_count = handshake->battle_frame_count();
  upd.emplace_back("battle_frame_count");
  replay = handshake->is_replay();
  upd.emplace_back("replay");

  postUpdate(upd);
  return upd;
}

std::vector<std::string> State::update(const TorchCraft::Frame* frame) {
  std::vector<std::string> upd;

  if (flatbuffers::IsFieldPresent(frame, TorchCraft::Frame::VT_DATA)) {
    frame_string.assign(frame->data()->begin(), frame->data()->end());
    std::istringstream ss(frame_string);
    ss >> *this->frame;
    upd.emplace_back("frame_string");
    upd.emplace_back("frame");
  }

  deaths.clear();
  upd.emplace_back("deaths");
  if (flatbuffers::IsFieldPresent(frame, TorchCraft::Frame::VT_DEATHS)) {
    deaths.assign(frame->deaths()->begin(), frame->deaths()->end());
  }

  frame_from_bwapi = frame->frame_from_bwapi();
  upd.emplace_back("frame_from_bwapi");
  battle_frame_count = frame->battle_frame_count();
  upd.emplace_back("battle_frame_count");

  if (flatbuffers::IsFieldPresent(frame, TorchCraft::Frame::VT_IMG_MODE)) {
    img_mode = frame->img_mode()->str();
    upd.emplace_back("img_mode");
  }

  if (flatbuffers::IsFieldPresent(
          frame, TorchCraft::Frame::VT_SCREEN_POSITION)) {
    screen_position[0] = frame->screen_position()->x();
    screen_position[1] = frame->screen_position()->y();
    upd.emplace_back("screen_position");
  }

  if (flatbuffers::IsFieldPresent(frame, TorchCraft::Frame::VT_VISIBILITY) &&
      flatbuffers::IsFieldPresent(
          frame, TorchCraft::Frame::VT_VISIBILITY_SIZE)) {
    if (frame->visibility()->size() ==
        frame->visibility_size()->x() * frame->visibility_size()->y()) {
      visibility_size[0] = frame->visibility_size()->x();
      visibility_size[1] = frame->visibility_size()->y();
      visibility.assign(
          frame->visibility()->begin(), frame->visibility()->end());
      upd.emplace_back("visibility");
    } else {
      visibility_size[0] = 0;
      visibility_size[1] = 0;
      visibility.clear();
      std::cerr << "Warning: visibility data does not match visibility size"
                << std::endl;
    }
  }

  if (flatbuffers::IsFieldPresent(frame, TorchCraft::Frame::VT_IMG_DATA) &&
      flatbuffers::IsFieldPresent(frame, TorchCraft::Frame::VT_IMG_SIZE)) {
    if (setRawImage(frame)) {
      upd.emplace_back("image");
    }
  }

  postUpdate(upd);
  return upd;
}

std::vector<std::string> State::update(const TorchCraft::EndGame* end) {
  std::vector<std::string> upd;

  if (flatbuffers::IsFieldPresent(end, TorchCraft::EndGame::VT_FRAME)) {
    frame_string.assign(end->frame()->begin(), end->frame()->end());
    std::istringstream ss(frame_string);
    ss >> *frame;
    upd.emplace_back("frame_string");
    upd.emplace_back("frame");
  }

  game_ended = true;
  upd.emplace_back("game_ended");
  game_won = end->game_won();
  upd.emplace_back("game_won");

  postUpdate(upd);
  return upd;
}

bool State::setRawImage(const TorchCraft::Frame* frame) {
  if (frame->img_data()->size() !=
      frame->img_size()->x() * frame->img_size()->y() * 4) {
    image_size[0] = 0;
    image_size[1] = 0;
    image.clear();
    std::cerr << "Warning: image data does not match image size" << std::endl;
    return false;
  }

  image_size[0] = frame->img_size()->x();
  image_size[1] = frame->img_size()->y();

  // Incoming binary data is [BGRA,...], which we transform into [R..,G..,B..].
  image.resize(image_size[0] * image_size[1] * 3);
  auto dst = image.begin();
  auto srcBegin = frame->img_data()->begin();
  for (int a = 2; a >= 0; --a) {
    auto src = srcBegin + a;
    for (int i = 0; i < image_size[0] * image_size[1]; i++) {
      *dst++ = *src;
      src += 4;
    }
  }

  return true;
}

void State::postUpdate(std::vector<std::string>& upd) {
  if (microBattles_) {
    if (battle_just_ended) {
      upd.emplace_back("battle_just_ended");
    }
    battle_just_ended = false;

    // Apply list of deaths on list of units from previous frame
    // so that battle ended condition can be detected every time.
    // This is particularly important with frame skipping: it's possible
    // that all remaining units die in the same interval and we wouldn't be able
    // to find out who won.
    for (auto d : deaths) {
      aliveUnits.erase(d);
      if (!onlyConsiderTypes_.empty()) {
        aliveUnitsConsidered.erase(d);
      }
      if (checkBattleFinished(upd)) {
        break;
      }
    }

    if (battle_just_ended) {
      // Remove dead units from *previous* frame; if the battle has just ended,
      // we won't bother copying the frame units.
      for (auto& us : units) {
        us.second.erase(
            std::remove_if(
                us.second.begin(),
                us.second.end(),
                [this](const replayer::Unit& unit) {
                  return aliveUnits.find(unit.id) == aliveUnits.end();
                }),
            us.second.end());
      }
    }
  }

  if (microBattles_ && battle_just_ended) {
    return;
  }

  // Update units
  for (const auto& fus : frame->units) {
    auto player = fus.first;
    if (units.find(player) == units.end()) {
      units.emplace(player, std::vector<replayer::Unit>());
    } else {
      units[player].clear();
    }

    std::copy_if(
        fus.second.begin(),
        fus.second.end(),
        std::back_inserter(units[player]),
        [this, player](const replayer::Unit& unit) {
          auto ut = client::BW::UnitType::_from_integral_nothrow(unit.type);
          return (
              // Unit is of known type (or enemy unit)
              (player != player_id || ut) &&
              // Unit has not been marked dead
              std::find(deaths.begin(), deaths.end(), unit.id) == deaths.end());
        });
  }

  // Update alive units
  aliveUnits.clear();
  aliveUnitsConsidered.clear();
  for (const auto& us : units) {
    auto player = us.first;
    for (const auto& unit : us.second) {
      aliveUnits[unit.id] = player;
      if (!onlyConsiderTypes_.empty() &&
          onlyConsiderTypes_.find(BW::UnitType::_from_integral(unit.type)) !=
              onlyConsiderTypes_.end()) {
        aliveUnitsConsidered[unit.id] = player;
      }
    }
  }

  if (microBattles_ && waiting_for_restart) {
    // Check if both players have active units
    auto numUnitsMyself = units[player_id].size();
    auto numUnitsEnemy = units[1 - player_id].size();
    if (numUnitsMyself > 0 && numUnitsEnemy > 0) {
      waiting_for_restart = false;
      upd.emplace_back("waiting_for_restart");
    }
  }
}

bool State::checkBattleFinished(std::vector<std::string>& upd) {
  if (waiting_for_restart) {
    return false;
  }

  auto map = &aliveUnits;
  if (!onlyConsiderTypes_.empty()) {
    map = &aliveUnitsConsidered;
  }
  size_t numUnitsMyself = 0;
  size_t numUnitsEnemy = 0;
  for (auto unit : *map) {
    if (unit.second == player_id) {
      numUnitsMyself++;
    } else if (unit.second == 1 - player_id) {
      numUnitsEnemy++;
    }
  }

  if (numUnitsMyself == 0 || numUnitsEnemy == 0) {
    battle_just_ended = true;
    upd.emplace_back("battle_just_ended");
    battle_won = numUnitsMyself > 0 || numUnitsEnemy == 0;
    upd.emplace_back("battle_won");
    waiting_for_restart = true;
    upd.emplace_back("waiting_for_restart");
    last_battle_ended = frame_from_bwapi;
    upd.emplace_back("last_battle_ended");
    return true;
  }
  return false;
}

} // namespace client
