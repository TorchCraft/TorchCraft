/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "state.h"

extern "C" {
#include <TH/TH.h>
#include <lauxlib.h>
#include <luaT.h>
#include <lualib.h>
}

namespace {

template <typename OutputIt>
void copyIntegerArray(lua_State* L, int index, OutputIt it, int max = -1) {
  lua_pushvalue(L, index);
  int i = 1;
  while (max < 0 || i <= max) {
    lua_rawgeti(L, -1, i);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
      break;
    }
    *it++ = luaL_checkint(L, -1);
    lua_pop(L, 1);
    i++;
  }
  lua_pop(L, 1);
}

template <typename OutputIt>
void copy2DIntegerArray(lua_State* L, int index, OutputIt it, int size[2]) {
  lua_pushvalue(L, index);

  int i = 1;
  int maxj = -1;
  while (true) {
    lua_rawgeti(L, -1, i);
    if (lua_isnil(L, -1) || !lua_istable(L, -1)) {
      lua_pop(L, 1);
      break;
    }

    int j = 1;
    while (true) {
      lua_rawgeti(L, -1, j);
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        break;
      }

      *it++ = luaL_checkint(L, -1);
      lua_pop(L, 1);
      j++;
    }

    lua_pop(L, 1);
    i++;

    if (maxj < 0) {
      maxj = j;
    } else if (maxj != j) {
      break;
    }
  }

  size[0] = i - 1;
  size[1] = maxj - 1;
  lua_pop(L, 1);
}

void copyMapData(
    lua_State* L,
    int index,
    std::vector<uint8_t>& dest,
    int size[2]) {
  THByteTensor* data =
      static_cast<THByteTensor*>(luaT_checkudata(L, index, "torch.ByteTensor"));
  assert(THByteTensor_nDimension(data) == 2);
  auto storage = data->storage;
  auto n = THByteStorage_size(storage);
  dest.resize(n);
  memcpy(dest.data(), storage->data, n);
  size[0] = THByteTensor_size(data, 0);
  size[1] = THByteTensor_size(data, 1);
}

} // namespace

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

std::string State::update(lua_State* L, const std::string& msg) {
  // Detect optional image in reply from server
  const char marker[] = "TCIMAGEDATA";
  auto pos = msg.find(marker);
  std::string update;
  if (pos != std::string::npos) {
    auto off = pos + sizeof(marker) - 1;
    auto imageMsg = msg.substr(off, msg.length() - off - 2);
    updateImage(imageMsg);
    update = msg.substr(0, pos) + "}";
  } else {
    update = msg;
  }

  // Parse Lua state update
  luaL_dostring(L, ("return " + update).c_str());

  // Iterate over update and apply changes
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    std::string key(lua_tostring(L, -2));
    if (key == "frame") {
      frame_string = luaL_checkstring(L, -1);
      std::istringstream ss(frame_string);
      ss >> *frame;
    } else if (key == "lag_frames") {
      lag_frames = luaL_checkint(L, -1);
    } else if (key == "map_data") {
      copyMapData(L, -1, map_data, map_data_size);
    } else if (key == "map_name") {
      map_name = luaL_checkstring(L, -1);
    } else if (key == "is_replay") {
      is_replay = lua_toboolean(L, -1);
    } else if (key == "player_id") {
      player_id = luaL_checkint(L, -1);
    } else if (key == "neutral_id") {
      neutral_id = luaL_checkint(L, -1);
    } else if (key == "deaths") {
      deaths.clear();
      copyIntegerArray(L, -1, std::inserter(deaths, deaths.begin()));
    } else if (key == "frame_from_bwapi") {
      frame_from_bwapi = luaL_checkint(L, -1);
    } else if (key == "battle_frame_count") {
      battle_frame_count = luaL_checkint(L, -1);
    } else if (key == "game_ended") {
      game_ended = lua_toboolean(L, -1);
    } else if (key == "game_won") {
      game_won = lua_toboolean(L, -1);
    } else if (key == "img_mode") {
      img_mode = luaL_checkstring(L, -1);
    } else if (key == "screen_position") {
      copyIntegerArray(L, -1, screen_position, 2);
    } else if (key == "visibility") {
      visibility.clear();
      copy2DIntegerArray(
          L,
          -1,
          std::inserter(visibility, visibility.begin()),
          visibility_size);
    } else {
      std::cerr << "!! unknown key: " << key << std::endl;
    }

    lua_pop(L, 1);
  }

  return update;
}

void State::updateImage(const std::string& msg) {
  std::istringstream ss(msg);
  std::string t;
  std::getline(ss, t, ',');
  auto width = std::stoi(t);
  std::getline(ss, t, ',');
  auto height = std::stoi(t);
  auto imgdata = msg.c_str() + ss.tellg();

  image.resize(3 * height * width);
  auto imgIt = image.begin();

  // Incoming binary data is [BGRA,...], which we transform into [R..,G..,B..].
  for (int a = 2; a >= 0; --a) {
    int it = a;
    for (int i = 0; i < height * width; i++) {
      *imgIt++ = imgdata[it];
      it += 4;
    }
  }

  image_size[0] = width;
  image_size[1] = height;
}

} // namespace client
