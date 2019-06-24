/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "state.h"

#include "messages_generated.h"

namespace fb = flatbuffers;

namespace torchcraft {

State::State(): RefCounted(), frame(new Frame()) {
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
      map_title(other.map_title),
      start_locations(other.start_locations),
      player_info(other.player_info),
      player_id(other.player_id),
      neutral_id(other.neutral_id),
      replay(other.replay),
      frame(new Frame(other.frame)),
      deaths(other.deaths),
      frame_from_bwapi(other.frame_from_bwapi),
      game_ended(other.game_ended),
      game_won(other.game_won),
      img_mode(other.img_mode),
      screen_position{other.screen_position[0], other.screen_position[1]},
      visibility(other.visibility),
      visibility_size{other.visibility_size[0], other.visibility_size[1]},
      image(other.image),
      image_size{other.image_size[0], other.image_size[1]},
      units(other.units),
      allUnits(other.allUnits),
      numUpdates(other.numUpdates) {}

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
  swap(a.map_title, b.map_title);
  swap(a.start_locations, b.start_locations);
  swap(a.player_info, b.player_info);
  swap(a.player_id, b.player_id);
  swap(a.neutral_id, b.neutral_id);
  swap(a.replay, b.replay);
  swap(a.frame, b.frame);
  swap(a.deaths, b.deaths);
  swap(a.frame_from_bwapi, b.frame_from_bwapi);
  swap(a.game_ended, b.game_ended);
  swap(a.game_won, b.game_won);
  swap(a.img_mode, b.img_mode);
  swap(a.screen_position[0], b.screen_position[0]);
  swap(a.screen_position[1], b.screen_position[1]);
  swap(a.visibility, b.visibility);
  swap(a.visibility_size[0], b.visibility_size[0]);
  swap(a.screen_position[1], b.screen_position[1]);
  swap(a.image, b.image);
  swap(a.image_size[0], b.image_size[0]);
  swap(a.image_size[1], b.image_size[1]);
  swap(a.units, b.units);
  swap(a.allUnits, b.allUnits);
  swap(a.numUpdates, b.numUpdates);
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
  map_title.clear();
  start_locations.clear();
  player_info.clear();
  frame->clear();
  deaths.clear();
  frame_from_bwapi = 0;
  game_ended = false;
  game_won = false;
  img_mode.clear();
  screen_position[0] = -1;
  screen_position[1] = -1;
  image.clear(); // XXX invalidates existing tensors pointing to it
  image_size[0] = 0;
  image_size[1] = 0;
  units.clear();
  units[-1]={};
  units[0]={};
  units[1]={};
  allUnits.clear();

  numUpdates++;
}

std::vector<std::string> State::update(const fbs::HandshakeServer* handshake) {
  reset();

  std::vector<std::string> upd;
  lag_frames = handshake->lag_frames();
  upd.emplace_back("lag_frames");
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_GROUND_HEIGHT_DATA)) {
    auto& ghd = *handshake->ground_height_data();
    ground_height_data.resize(ghd.size());
    for (size_t i = 0; i < ghd.size(); i++) {
      ground_height_data[i] = ghd[i];
    }
    upd.emplace_back("ground_height_data");
  }
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_WALKABLE_DATA)) {
    auto& wd = *handshake->walkable_data();
    walkable_data.resize(wd.size());
    for (size_t i = 0; i < wd.size(); i++) {
      walkable_data[i] = wd[i];
    }
    upd.emplace_back("walkable_data");
  }
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_BUILDABLE_DATA)) {
    auto& bd = *handshake->buildable_data();
    buildable_data.resize(bd.size());
    for (size_t i = 0; i < bd.size(); i++) {
      buildable_data[i] = bd[i];
    }
    upd.emplace_back("buildable_data");
  }
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_MAP_SIZE)) {
    map_size[0] = handshake->map_size()->x();
    map_size[1] = handshake->map_size()->y();
  }
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_MAP_NAME)) {
    map_name = handshake->map_name()->str();
    upd.emplace_back("map_name");
  }
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_MAP_TITLE)) {
    map_title = handshake->map_title()->str();
    upd.emplace_back("map_title");
  }
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_START_LOCATIONS)) {
    start_locations.clear();
    for (auto p : *handshake->start_locations()) {
      start_locations.emplace_back(p->x(), p->y());
    }
    upd.emplace_back("start_locations");
  }
  if (fb::IsFieldPresent(handshake, fbs::HandshakeServer::VT_PLAYERS)) {
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
  replay = handshake->is_replay();
  upd.emplace_back("replay");
  frame_from_bwapi = handshake->frame_from_bwapi();
  upd.emplace_back("frame_from_bwapi");

  postUpdate(upd);
  return upd;
}

bool State::update_frame(const void* flatBuffer, const fbs::FrameOrFrameDiff type) {
  switch (type) {
    case fbs::FrameOrFrameDiff::Frame: {
      auto frameFlatBuffer = static_cast<const fbs::Frame*>(flatBuffer);
      frame->readFromFlatBufferTable(*frameFlatBuffer);
      return true;
    }
    case fbs::FrameOrFrameDiff::FrameDiff:  {
      auto frameDiffFlatBuffer = static_cast<const fbs::FrameDiff*>(flatBuffer);
      replayer::FrameDiff frameDiff;
      frameDiff.readFromFlatBufferTable(*frameDiffFlatBuffer);
      replayer::frame_undiff(frame, frame, &frameDiff);
      return true;
    }
    default:
      throw std::runtime_error(
        "State::update_frame(): Unrecognized FrameOrFrameDiff type"); 
  }
}

std::vector<std::string> State::update(const fbs::StateUpdate* stateUpdate) {
  std::vector<std::string> upd;
  preUpdate();

  if (fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_DATA)) {
    if (this->update_frame(
      stateUpdate->data(),
      stateUpdate->data_type())) {
      upd.emplace_back("frame");
    }
  }

  if (fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_DEATHS)) {
    auto& fd = *stateUpdate->deaths();
    deaths.resize(fd.size());
    for (size_t i = 0; i < fd.size(); i++) {
      deaths[i] = fd[i];
    }
    if (!deaths.empty()) {
      upd.emplace_back("deaths");
    }
  }

  frame_from_bwapi = stateUpdate->frame_from_bwapi();
  upd.emplace_back("frame_from_bwapi");

  if (fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_IMG_MODE)) {
    img_mode = stateUpdate->img_mode()->str();
    upd.emplace_back("img_mode");
  }

  if (fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_SCREEN_POSITION)) {
    screen_position[0] = stateUpdate->screen_position()->x();
    screen_position[1] = stateUpdate->screen_position()->y();
    upd.emplace_back("screen_position");
  }

  if (fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_VISIBILITY) &&
      fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_VISIBILITY_SIZE)) {
    if (stateUpdate->visibility()->size() ==
        static_cast<size_t>(
            stateUpdate->visibility_size()->x() * stateUpdate->visibility_size()->y())) {
      visibility_size[0] = stateUpdate->visibility_size()->x();
      visibility_size[1] = stateUpdate->visibility_size()->y();
      auto& vb = *stateUpdate->visibility();
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

  if (fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_IMG_DATA) &&
      fb::IsFieldPresent(stateUpdate, fbs::StateUpdate::VT_IMG_SIZE)) {
    if (setRawImage(stateUpdate)) {
      upd.emplace_back("image");
    }
  }

  postUpdate(upd);
  return upd;
}

std::vector<std::string> State::update(const fbs::EndGame* end) {
  std::vector<std::string> upd;
  preUpdate();

  if (fb::IsFieldPresent(end, fbs::EndGame::VT_DATA)) {
    if (this->update_frame(
      end->data(),
      end->data_type())) {
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
    const fbs::PlayerLeft* left) {
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

bool State::setRawImage(const fbs::StateUpdate* frame) {
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

  // Update units
  for (auto& us : units) {
    if (frame->units.find(us.first) == frame->units.end()) {
      // No more units from this team
      us.second.clear();
    }
  }
  for (const auto& frameUnits : frame->units) {
    auto player = frameUnits.first;
    if (units.find(player) == units.end()) {
      units.emplace(player, std::vector<Unit>());
    } else {
      units[player].clear();
    }

    std::copy_if(
        frameUnits.second.begin(),
        frameUnits.second.end(),
        std::back_inserter(units[player]),
        [this, player](const Unit& unit) {
          auto ut = torchcraft::BW::UnitType::_from_integral_nothrow(unit.type);
          return (
              // Unit is of known type (or a neutral unit)
              (player == neutral_id || ut) &&
              // Unit has not been marked dead
              std::find(deaths.begin(), deaths.end(), unit.id) == deaths.end());
        });
  }
  for (auto& playerUnits : units) { 
    for (auto& unit : playerUnits.second) {
      allUnits.push_back(&unit);
    }
  }
}

} // namespace torchcraft
