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

State::State() : RefCounted(), frame(new replayer::Frame()) {
  reset();
}

State::~State() {
  frame->decref();
}

void State::reset() {
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
  img_mode.clear();
  screen_position[0] = -1;
  screen_position[1] = -1;
  image.clear(); // XXX invalidates existing tensors pointing to it
  image_size[0] = 0;
  image_size[1] = 0;
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
  // TODO: is_replay
  player_id = handshake->player_id();
  upd.emplace_back("player_id");
  neutral_id = handshake->neutral_id();
  upd.emplace_back("neutral_id");
  battle_frame_count = handshake->battle_frame_count();
  upd.emplace_back("battle_frame_count");
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

} // namespace client
