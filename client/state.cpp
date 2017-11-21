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

namespace torchcraft {

State::State(bool microBattles, std::set<BW::UnitType> onlyConsiderTypes)
    : RefCounted(),
      frame(new Frame()),
      microBattles_(microBattles),
      onlyConsiderTypes_(std::move(onlyConsiderTypes)) {
  reset();
}

State::State(const State& other)
    : RefCounted(),
      lag_frames(other.lag_frames),
      map_size{other.map_size[0], other.map_size[1]},
      ground_height_data(other.ground_height_data),
      walkable_data(other.walkable_data),
      buildable_data(other.buildable_data),
      map_name(other.map_name),
      start_locations(other.start_locations),
      player_info(other.player_info),
      player_id(other.player_id),
      neutral_id(other.neutral_id),
      replay(other.replay),
      frame(new Frame(other.frame)),
      deaths(other.deaths),
      frame_from_bwapi(other.frame_from_bwapi),
      battle_frame_count(other.battle_frame_count),
      game_ended(other.game_ended),
      game_won(other.game_won),
      battle_just_ended(other.battle_just_ended),
      battle_won(other.battle_won),
      waiting_for_restart(other.waiting_for_restart),
      last_battle_ended(other.last_battle_ended),
      img_mode(other.img_mode),
      screen_position{other.screen_position[0], other.screen_position[1]},
      visibility(other.visibility),
      visibility_size{other.visibility_size[0], other.visibility_size[1]},
      image(other.image),
      image_size{other.image_size[0], other.image_size[1]},
      aliveUnits(other.aliveUnits),
      aliveUnitsConsidered(other.aliveUnitsConsidered),
      units(other.units),
      numUpdates(other.numUpdates),
      microBattles_(other.microBattles_),
      onlyConsiderTypes_(other.onlyConsiderTypes_) {}

State::State(State&& other) : RefCounted(), frame(nullptr) {
  swap(*this, other);
}

State::~State() {
  if (frame) {
    frame->decref();
  }
}

State& State::operator=(State other) {
  swap(*this, other);
  return *this;
}

void swap(State& a, State& b) {
  using std::swap;
  swap(a.lag_frames, b.lag_frames);
  swap(a.map_size[0], b.map_size[0]);
  swap(a.map_size[1], b.map_size[1]);
  swap(a.ground_height_data, b.ground_height_data);
  swap(a.walkable_data, b.walkable_data);
  swap(a.buildable_data, b.buildable_data);
  swap(a.map_name, b.map_name);
  swap(a.start_locations, b.start_locations);
  swap(a.player_info, b.player_info);
  swap(a.player_id, b.player_id);
  swap(a.neutral_id, b.neutral_id);
  swap(a.replay, b.replay);
  swap(a.frame, b.frame);
  swap(a.deaths, b.deaths);
  swap(a.frame_from_bwapi, b.frame_from_bwapi);
  swap(a.battle_frame_count, b.battle_frame_count);
  swap(a.game_ended, b.game_ended);
  swap(a.game_won, b.game_won);
  swap(a.battle_just_ended, b.battle_just_ended);
  swap(a.battle_won, b.battle_won);
  swap(a.waiting_for_restart, b.waiting_for_restart);
  swap(a.last_battle_ended, b.last_battle_ended);
  swap(a.img_mode, b.img_mode);
  swap(a.screen_position[0], b.screen_position[0]);
  swap(a.screen_position[1], b.screen_position[1]);
  swap(a.visibility, b.visibility);
  swap(a.visibility_size[0], b.visibility_size[0]);
  swap(a.screen_position[1], b.screen_position[1]);
  swap(a.image, b.image);
  swap(a.image_size[0], b.image_size[0]);
  swap(a.image_size[1], b.image_size[1]);
  swap(a.aliveUnits, b.aliveUnits);
  swap(a.aliveUnitsConsidered, b.aliveUnitsConsidered);
  swap(a.units, b.units);
  swap(a.numUpdates, b.numUpdates);
  swap(a.microBattles_, b.microBattles_);
  swap(a.onlyConsiderTypes_, b.onlyConsiderTypes_);
}

void State::reset() {
  replay = false;
  lag_frames = 0;
  map_size[0] = 0;
  map_size[1] = 0;
  ground_height_data.clear();
  walkable_data.clear();
  buildable_data.clear();
  map_name.clear();
  start_locations.clear();
  player_info.clear();
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

  numUpdates++;
}

std::vector<std::string> State::update(
    const torchcraft::fbs::HandshakeServer* handshake) {
  reset();

  std::vector<std::string> upd;
  lag_frames = handshake->lag_frames();
  upd.emplace_back("lag_frames");
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeServer::VT_GROUND_HEIGHT_DATA)) {
    auto& ghd = *handshake->ground_height_data();
    ground_height_data.resize(ghd.size());
    for (size_t i = 0; i < ghd.size(); i++) {
      ground_height_data[i] = ghd[i];
    }
    upd.emplace_back("ground_height_data");
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeServer::VT_WALKABLE_DATA)) {
    auto& wd = *handshake->walkable_data();
    walkable_data.resize(wd.size());
    for (size_t i = 0; i < wd.size(); i++) {
      walkable_data[i] = wd[i];
    }
    upd.emplace_back("walkable_data");
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeServer::VT_BUILDABLE_DATA)) {
    auto& bd = *handshake->buildable_data();
    buildable_data.resize(bd.size());
    for (size_t i = 0; i < bd.size(); i++) {
      buildable_data[i] = bd[i];
    }
    upd.emplace_back("buildable_data");
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeServer::VT_MAP_SIZE)) {
    map_size[0] = handshake->map_size()->x();
    map_size[1] = handshake->map_size()->y();
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeServer::VT_MAP_NAME)) {
    map_name = handshake->map_name()->str();
    upd.emplace_back("map_name");
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeServer::VT_START_LOCATIONS)) {
    start_locations.clear();
    for (auto p : *handshake->start_locations()) {
      start_locations.emplace_back(p->x(), p->y());
    }
    upd.emplace_back("start_locations");
  }
  if (flatbuffers::IsFieldPresent(
          handshake, torchcraft::fbs::HandshakeServer::VT_PLAYERS)) {
    player_info.clear();
    for (auto player : *handshake->players()) {
      PlayerInfo info;
      info.id = player->id();
      auto bwrace = BW::Race::_from_integral_nothrow(player->race());
      info.race = bwrace ? *bwrace : +BW::Race::Unknown;
      info.name = player->name() ? player->name()->str() : "";
      info.is_enemy = player->is_enemy();
      info.has_left = false;
      player_info[info.id] = info;
    }
    upd.emplace_back("players");
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

bool State::update_frame(const torchcraft::fbs::FrameData* fd) {
  if (fd->data() == nullptr)
    return false;
  if (fd->is_diff()) {
    std::istringstream ss(std::string(
        reinterpret_cast<const char*>(fd->data()->data()), fd->data()->size()));
    replayer::FrameDiff diff;
    ss >> diff;
    replayer::frame_undiff(this->frame, this->frame, &diff);
  } else {
    auto frame_string = std::string(
        reinterpret_cast<const char*>(fd->data()->data()), fd->data()->size());
    std::istringstream ss(frame_string);
    ss >> *this->frame;
  }
  return true;
}

std::vector<std::string> State::update(const torchcraft::fbs::Frame* frame) {
  std::vector<std::string> upd;
  preUpdate();

  if (flatbuffers::IsFieldPresent(frame, torchcraft::fbs::Frame::VT_DATA)) {
    if (this->update_frame(frame->data())) {
      upd.emplace_back("frame");
    }
  }

  if (flatbuffers::IsFieldPresent(frame, torchcraft::fbs::Frame::VT_DEATHS)) {
    auto& fd = *frame->deaths();
    deaths.resize(fd.size());
    for (size_t i = 0; i < fd.size(); i++) {
      deaths[i] = fd[i];
    }
    if (!deaths.empty()) {
      upd.emplace_back("deaths");
    }
  }

  frame_from_bwapi = frame->frame_from_bwapi();
  upd.emplace_back("frame_from_bwapi");
  battle_frame_count = frame->battle_frame_count();
  upd.emplace_back("battle_frame_count");

  if (flatbuffers::IsFieldPresent(frame, torchcraft::fbs::Frame::VT_IMG_MODE)) {
    img_mode = frame->img_mode()->str();
    upd.emplace_back("img_mode");
  }

  if (flatbuffers::IsFieldPresent(
          frame, torchcraft::fbs::Frame::VT_SCREEN_POSITION)) {
    screen_position[0] = frame->screen_position()->x();
    screen_position[1] = frame->screen_position()->y();
    upd.emplace_back("screen_position");
  }

  if (flatbuffers::IsFieldPresent(
          frame, torchcraft::fbs::Frame::VT_VISIBILITY) &&
      flatbuffers::IsFieldPresent(
          frame, torchcraft::fbs::Frame::VT_VISIBILITY_SIZE)) {
    if (frame->visibility()->size() ==
        static_cast<size_t>(
            frame->visibility_size()->x() * frame->visibility_size()->y())) {
      visibility_size[0] = frame->visibility_size()->x();
      visibility_size[1] = frame->visibility_size()->y();
      auto& vb = *frame->visibility();
      visibility.resize(vb.size());
      for (size_t i = 0; i < vb.size(); i++) {
        visibility[i] = vb[i];
      }
      upd.emplace_back("visibility");
    } else {
      visibility_size[0] = 0;
      visibility_size[1] = 0;
      visibility.clear();
      std::cerr << "Warning: visibility data does not match visibility size"
                << std::endl;
    }
  }

  if (flatbuffers::IsFieldPresent(frame, torchcraft::fbs::Frame::VT_IMG_DATA) &&
      flatbuffers::IsFieldPresent(frame, torchcraft::fbs::Frame::VT_IMG_SIZE)) {
    if (setRawImage(frame)) {
      upd.emplace_back("image");
    }
  }

  postUpdate(upd);
  return upd;
}

std::vector<std::string> State::update(const torchcraft::fbs::EndGame* end) {
  std::vector<std::string> upd;
  preUpdate();

  if (flatbuffers::IsFieldPresent(end, torchcraft::fbs::EndGame::VT_DATA)) {
    if (this->update_frame(end->data())) {
      upd.emplace_back("frame");
    }
  }

  game_ended = true;
  upd.emplace_back("game_ended");
  game_won = end->game_won();
  upd.emplace_back("game_won");

  postUpdate(upd);
  return upd;
}

std::vector<std::string> State::update(
    const torchcraft::fbs::PlayerLeft* left) {
  preUpdate();
  if (left->player_left()) {
    for (auto& it : player_info) {
      if (it.second.name == left->player_left()->str()) {
        it.second.has_left = true;
        break;
      }
    }
  }
  return std::vector<std::string>();
}

std::vector<std::string> State::update(const torchcraft::fbs::Error* error) {
  preUpdate();
  return std::vector<std::string>();
}

bool State::setRawImage(const torchcraft::fbs::Frame* frame) {
  if (frame->img_data()->size() !=
      static_cast<size_t>(
          frame->img_size()->x() * frame->img_size()->y() * 4)) {
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

void State::preUpdate() {
  deaths.clear();
}

void State::postUpdate(std::vector<std::string>& upd) {
  numUpdates++;

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
                [this](const Unit& unit) {
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
      units.emplace(player, std::vector<Unit>());
    } else {
      units[player].clear();
    }

    std::copy_if(
        fus.second.begin(),
        fus.second.end(),
        std::back_inserter(units[player]),
        [this, player](const Unit& unit) {
          auto ut = torchcraft::BW::UnitType::_from_integral_nothrow(unit.type);
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

} // namespace torchcraft
