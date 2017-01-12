/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <string>
#include <vector>

extern "C" {
#include <lua.h>
}

#include "replayer/frame.h"
#include "replayer/refcount.h"

namespace client {

class State : public RefCounted {
 public:
  // setup
  int lag_frames; // number of frames from order to execution
  std::vector<uint8_t> map_data; // 2D. 255 where not available
  int map_data_size[2];
  std::string map_name; // Name on the current map
  bool is_replay;
  int player_id;
  int neutral_id;

  // game state
  replayer::Frame* frame; // this will allow for easy reset (XXX)
  std::string frame_string;
  std::vector<int> deaths;
  int frame_from_bwapi;
  int battle_frame_count; // if micro mode

  bool game_ended; // did the game end?
  bool game_won;

  // if with image
  std::string img_mode;
  int screen_position[2]; // position of screen {x, y} in pixels. {0, 0} is
  // top-left
  std::vector<uint8_t> visibility;
  int visibility_size[2];
  std::vector<uint8_t> image; // RGB
  int image_size[2];

  State();
  ~State();

  void reset();
  std::string update(lua_State* L, const std::string& upd);

 private:
  void updateImage(const std::string& msg);
};

} // namespace client
