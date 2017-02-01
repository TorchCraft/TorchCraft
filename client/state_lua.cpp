/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <set>

#include "replayer/frame_lua.h"
#include "state_lua.h"

namespace {

inline client::State* checkState(lua_State* L, int index = 1) {
  auto s = luaL_checkudata(L, index, "torchcraft.State");
  luaL_argcheck(L, s != nullptr, index, "'state' expected");
  return *static_cast<client::State**>(s);
}

int push2DIntegerArray(
    lua_State* L,
    const std::vector<uint8_t>& data,
    int size[2]) {
  auto it = data.begin();
  lua_createtable(L, size[1], 0);
  for (int i = 0; i < size[1]; i++) {
    lua_createtable(L, size[0], 0);
    for (int j = 0; j < size[0]; j++) {
      lua_pushinteger(L, *it++);
      lua_rawseti(L, -2, j + 1);
    }
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}

const std::set<std::string> stateMembers = {
    "lag_frames",
    "map_data",
    "map_name",
    "player_id",
    "neutral_id",
    "frame",
    "frame_string",
    "deaths",
    "frame_from_bwapi",
    "battle_frame_count",
    "game_ended",
    "game_won",
    "img_mode",
    "screen_position",
    "visibility",
    "image",
};

// index represents the index of the state userdata on the stack
int pushMember(
    lua_State* L,
    client::State* s,
    const std::string& m,
    int index = 1) {
  if (m == "lag_frames") {
    lua_pushinteger(L, s->lag_frames);
  } else if (m == "map_data") {
    if (!s->map_data.empty()) {
      auto s0 = s->map_data_size[0];
      auto s1 = s->map_data_size[1];
      auto storage = THByteStorage_newWithData(s->map_data.data(), s0 * s1);
      THByteStorage_clearFlag(storage, TH_STORAGE_RESIZABLE);
      THByteStorage_clearFlag(storage, TH_STORAGE_FREEMEM);
      auto tensor = THByteTensor_newWithStorage2d(storage, 0, s0, s1, s1, 1);
      luaT_pushudata(L, (void*)tensor, "torch.ByteTensor");
    } else {
      lua_pushnil(L);
    }
  } else if (m == "map_name") {
    lua_pushstring(L, s->map_name.c_str());
  } else if (m == "player_id") {
    lua_pushinteger(L, s->player_id);
  } else if (m == "neutral_id") {
    lua_pushinteger(L, s->neutral_id);
  } else if (m == "frame") {
    auto f = (replayer::Frame**)lua_newuserdata(L, sizeof(replayer::Frame*));
    *f = s->frame;
    (*f)->incref();
    luaL_getmetatable(L, "torchcraft.Frame");
    lua_setmetatable(L, -2);
  } else if (m == "frame_string") {
    lua_pushlstring(L, s->frame_string.c_str(), s->frame_string.length());
  } else if (m == "deaths") {
    lua_createtable(L, s->deaths.size(), 0);
    int n = 1;
    for (auto d : s->deaths) {
      lua_pushinteger(L, d);
      lua_rawseti(L, -2, n++);
    }
  } else if (m == "frame_from_bwapi") {
    lua_pushinteger(L, s->frame_from_bwapi);
  } else if (m == "battle_frame_count") {
    lua_pushinteger(L, s->battle_frame_count);
  } else if (m == "game_ended") {
    lua_pushboolean(L, s->game_ended);
  } else if (m == "game_won") {
    lua_pushboolean(L, s->game_won);
  } else if (m == "img_mode") {
    lua_pushstring(L, s->img_mode.c_str());
  } else if (m == "screen_position") {
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, s->screen_position[0]);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, s->screen_position[1]);
    lua_rawseti(L, -2, 2);
  } else if (m == "visibility") {
    push2DIntegerArray(L, s->visibility, s->visibility_size);
  } else if (m == "image") {
    if (!s->image.empty()) {
      auto s0 = s->image_size[0];
      auto s1 = s->image_size[1];
      auto storage = THByteStorage_newWithData(s->image.data(), 3 * s0 * s1);
      THByteStorage_clearFlag(storage, TH_STORAGE_RESIZABLE);
      THByteStorage_clearFlag(storage, TH_STORAGE_FREEMEM);
      auto tensor =
          THByteTensor_newWithStorage3d(storage, 0, 3, s0 * s1, s1, s0, s0, 1);
      luaT_pushudata(L, (void*)tensor, "torch.ByteTensor");
    } else {
      lua_pushnil(L);
    }
  } else {
    lua_getuservalue(L, index);
    lua_getfield(L, -1, m.c_str());
    lua_remove(L, -2);
  }
  return 1;
}

} // namespace

int newState(lua_State* L) {
  return pushState(L);
}

int pushState(lua_State* L, client::State* state) {
  auto s = (client::State**)lua_newuserdata(L, sizeof(client::State*));
  if (state == nullptr) {
    *s = new client::State();
  } else {
    *s = state;
    state->incref();
  }

  luaL_getmetatable(L, "torchcraft.State");
  lua_setmetatable(L, -2);

  lua_newtable(L);
  lua_setuservalue(L, -2);
  return 1;
}

int freeState(lua_State* L) {
  auto s = checkState(L);
  s->decref();
  return 0;
}

int gcState(lua_State* L) {
  auto s =
      static_cast<client::State**>(luaL_checkudata(L, 1, "torchcraft.State"));
  assert(*s != nullptr);
  (*s)->decref();
  *s = nullptr;
  return 0;
}

int indexState(lua_State* L) {
  auto s = checkState(L);
  auto key = luaL_checkstring(L, 2);

  if (luaL_getmetafield(L, 1, key)) {
    if (!lua_isnil(L, -1) && lua_iscfunction(L, -1)) {
      return 1;
    }
    lua_pop(L, 1);
  }

  return pushMember(L, s, key);
}

int newindexState(lua_State* L) {
  auto s = checkState(L);
  auto key = luaL_checkstring(L, 2);

  if (stateMembers.find(key) != stateMembers.end()) {
    return luaL_error(L, ("Cannot overwrite key " + std::string(key)).c_str());
  }
  lua_getuservalue(L, 1);
  lua_pushvalue(L, 2);
  lua_pushvalue(L, 3);
  lua_settable(L, -3);
  lua_pop(L, 1);
  return 0;
}

int resetState(lua_State* L) {
  auto s = checkState(L);
  s->reset();

  // TODO: manage these things in State
  lua_getuservalue(L, 1);
  for (auto f : std::vector<const char*>({"units",
                                          "resources",
                                          "units_myself",
                                          "units_enemy",
                                          "resources_myself"})) {
    lua_newtable(L);
    lua_setfield(L, -2, f);
  }
  lua_pop(L, 1);

  return 0;
}

int totableState(lua_State* L) {
  auto s = checkState(L);
  lua_newtable(L);

  // Add members that are always present
  for (auto m : stateMembers) {
    pushMember(L, s, m);
    lua_setfield(L, -2, m.c_str());
  }

  // Add user values
  lua_getuservalue(L, 1);
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    lua_pushvalue(L, -2);
    lua_insert(L, -2);
    lua_settable(L, -5);
  }
  lua_pop(L, 1);
  return 1;
}

int pushUpdatesState(
    lua_State* L,
    std::vector<std::string>& updates,
    int index) {
  auto s = checkState(L, index);
  lua_newtable(L);
  for (auto u : updates) {
    pushMember(L, s, u);
    lua_setfield(L, -2, u.c_str());
  }
  return 1;
}

namespace client {
void registerState(lua_State* L, int index) {
  luaT_newlocalmetatable(
      L, "torchcraft.State", nullptr, ::newState, ::freeState, nullptr, index);
  luaL_newmetatable(L, "torchcraft.State");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaT_setfuncs(L, ::state_m, 0);
  lua_setfield(L, -2, "State");
  lua_pop(L, 1);
}
}
