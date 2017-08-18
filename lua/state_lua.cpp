/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <set>

#include "constants.h"
#include "lua_utils.h"
#include "state_lua.h"

#include "frame_lua.h"

namespace {

inline torchcraft::State* checkState(lua_State* L, int index = 1) {
  auto s = luaL_checkudata(L, index, "torchcraft.State");
  luaL_argcheck(L, s != nullptr, index, "'state' expected");
  return *static_cast<torchcraft::State**>(s);
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
    "ground_height_data",
    "walkable_data",
    "buildable_data",
    "map_name",
    "start_locations",
    "player_races",
    "player_names",
    "player_id",
    "neutral_id",
    "replay",
    "frame",
    "frame_string",
    "deaths",
    "frame_from_bwapi",
    "battle_frame_count",
    "game_ended",
    "game_won",
    "battle_just_ended",
    "battle_won",
    "waiting_for_restart",
    "last_battle_ended",
    "img_mode",
    "screen_position",
    "visibility",
    "image",
    "micro_battles",
    "only_consider_types",
};

void updateUserValueAndPush(
    lua_State* L,
    torchcraft::State* s,
    const std::string& m,
    int index);

// index represents the index of the state userdata on the stack
int pushMember(
    lua_State* L,
    torchcraft::State* s,
    const std::string& m,
    int index = 1) {
  if (m == "lag_frames") {
    lua_pushinteger(L, s->lag_frames);
  } else if (m == "ground_height_data") {
    if (!s->ground_height_data.empty()) {
      auto s0 = s->map_size[0];
      auto s1 = s->map_size[1];
      auto storage =
          THByteStorage_newWithData(s->ground_height_data.data(), s0 * s1);
      THByteStorage_clearFlag(storage, TH_STORAGE_RESIZABLE);
      THByteStorage_clearFlag(storage, TH_STORAGE_FREEMEM);
      auto tensor = THByteTensor_newWithStorage2d(storage, 0, s1, s0, s0, 1);
      luaT_pushudata(L, (void*)tensor, "torch.ByteTensor");
    } else {
      lua_pushnil(L);
    }
  } else if (m == "walkable_data") {
    if (!s->walkable_data.empty()) {
      auto s0 = s->map_size[0];
      auto s1 = s->map_size[1];
      auto storage =
          THByteStorage_newWithData(s->walkable_data.data(), s0 * s1);
      THByteStorage_clearFlag(storage, TH_STORAGE_RESIZABLE);
      THByteStorage_clearFlag(storage, TH_STORAGE_FREEMEM);
      auto tensor = THByteTensor_newWithStorage2d(storage, 0, s1, s0, s0, 1);
      luaT_pushudata(L, (void*)tensor, "torch.ByteTensor");
    } else {
      lua_pushnil(L);
    }
  } else if (m == "buildable_data") {
    if (!s->buildable_data.empty()) {
      auto s0 = s->map_size[0];
      auto s1 = s->map_size[1];
      auto storage =
          THByteStorage_newWithData(s->buildable_data.data(), s0 * s1);
      THByteStorage_clearFlag(storage, TH_STORAGE_RESIZABLE);
      THByteStorage_clearFlag(storage, TH_STORAGE_FREEMEM);
      auto tensor = THByteTensor_newWithStorage2d(storage, 0, s1, s0, s0, 1);
      luaT_pushudata(L, (void*)tensor, "torch.ByteTensor");
    } else {
      lua_pushnil(L);
    }
  } else if (m == "map_name") {
    lua_pushstring(L, s->map_name.c_str());
  } else if (m == "start_locations") {
    lua_createtable(L, s->start_locations.size(), 0);
    int n = 1;
    for (auto d : s->start_locations) {
      lua_createtable(L, 2, 0);
      lua_pushinteger(L, d.x);
      lua_setfield(L, -2, "x");
      lua_pushinteger(L, d.y);
      lua_setfield(L, -2, "y");
      lua_rawseti(L, -2, n++);
    }
  } else if (m == "player_races") {
    lua_createtable(L, 0, s->player_races.size());
    int n = 0;  // n=0 because player_ids start at 0
    for (auto pr : s->player_races) {
      lua_pushinteger(L, pr);
      lua_rawseti(L, -2, n++);
    }
  } else if (m == "player_names") {
    lua_createtable(L, 0, s->player_names.size());
    int n = 0;  // n=0 because player_ids start at 0
    for (auto pr : s->player_names) {
      lua_pushstring(L, pr.c_str());
      lua_rawseti(L, -2, n++);
    }
  } else if (m == "player_id") {
    lua_pushinteger(L, s->player_id);
  } else if (m == "neutral_id") {
    lua_pushinteger(L, s->neutral_id);
  } else if (m == "replay") {
    lua_pushboolean(L, s->replay);
  } else if (m == "frame") {
    auto f =
        (torchcraft::Frame**)lua_newuserdata(L, sizeof(torchcraft::Frame*));
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
  } else if (m == "battle_just_ended") {
    lua_pushboolean(L, s->battle_just_ended);
  } else if (m == "battle_won") {
    lua_pushboolean(L, s->battle_won);
  } else if (m == "waiting_for_restart") {
    lua_pushboolean(L, s->waiting_for_restart);
  } else if (m == "last_battle_ended") {
    lua_pushinteger(L, s->last_battle_ended);
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
  } else if (m == "micro_battles") {
    lua_pushboolean(L, s->microBattles());
  } else if (m == "only_consider_types") {
    auto types = s->onlyConsiderTypes();
    lua_createtable(L, types.size(), 0);
    int i = 1;
    for (auto ut : types) {
      torchcraft::lua::pushValue(L, ut._to_integral());
      lua_rawseti(L, -2, i++);
    }
  } else {
    updateUserValueAndPush(L, s, m, index);
  }
  return 1;
}

void pushFrameMember(
    lua_State* L,
    torchcraft::State* s,
    lua_CFunction f,
    int player) {
  lua_pushcfunction(L, f);
  pushMember(L, s, "frame");
  lua_pushinteger(L, player);
  lua_call(L, 2, 1);
}

void pushUnits(lua_State* L, const std::vector<torchcraft::Unit>& units) {
  lua_createtable(L, units.size(), 0);
  for (const auto& u : units) {
    pushUnit(L, u);
    lua_rawseti(L, -2, u.id);
  }
}

void updateUserValueAndPush(
    lua_State* L,
    torchcraft::State* s,
    const std::string& m,
    int index) {
  lua_getuservalue(L, index);

  // Still up to date?
  lua_getfield(L, -1, "__numUpdates");
  if (lua_tointeger(L, -1) == s->numUpdates) {
    lua_pop(L, 1);
    lua_getfield(L, -1, m.c_str());
    lua_remove(L, -2);
    return;
  }
  lua_pop(L, 1);

  // Update
  auto myself = s->player_id;
  if (!s->replay) {
    pushUnits(L, s->units[myself]);
    lua_setfield(L, -2, "units_myself");
    pushUnits(L, s->units[1 - myself]);
    lua_setfield(L, -2, "units_enemy");
    pushFrameMember(L, s, frameGetResources, myself);
    lua_setfield(L, -2, "resources_myself");
  } else { // if replay
    auto nplayers = s->units.size();
    lua_newtable(L);
    for (size_t p = 0; p < nplayers; p++) {
      if (p != s->neutral_id) {
        pushUnits(L, s->units[p]);
        lua_rawseti(L, -2, p);
      }
    }
    lua_setfield(L, -2, "units");

    lua_newtable(L);
    for (size_t p = 0; p < nplayers; p++) {
      if (p != s->neutral_id) {
        pushFrameMember(L, s, frameGetResources, p);
        lua_rawseti(L, -2, p);
      }
    }
    lua_setfield(L, -2, "resources");
  }

  pushUnits(L, s->units[s->neutral_id]);
  lua_setfield(L, -2, "units_neutral");

  lua_pushinteger(L, s->numUpdates);
  lua_setfield(L, -2, "__numUpdates");

  // Access requested uservalue member
  lua_getfield(L, -1, m.c_str());
  lua_remove(L, -2);
}

} // namespace

int newState(lua_State* L) {
  return pushState(L);
}

int pushState(lua_State* L, torchcraft::State* state, bool copy) {
  auto s = (torchcraft::State**)lua_newuserdata(L, sizeof(torchcraft::State*));
  if (state == nullptr) {
    if (lua_gettop(L) == 1) {
      *s = new torchcraft::State();
    } else if (lua_gettop(L) == 2) {
      *s = new torchcraft::State(lua_toboolean(L, 2));
    } else {
      *s = new torchcraft::State(
          lua_toboolean(L, 2), torchcraft::getConsideredTypes(L, 3));
    }
  } else if (!copy) {
    *s = state;
    state->incref();
  } else {
    *s = new torchcraft::State(*state);
  }

  luaL_getmetatable(L, "torchcraft.State");
  lua_setmetatable(L, -2);

  lua_newtable(L);
  lua_pushinteger(L, -1);
  lua_setfield(L, -2, "__numUpdates");
  lua_setuservalue(L, -2);
  return 1;
}

int freeState(lua_State* L) {
  auto s = checkState(L);
  s->decref();
  return 0;
}

int gcState(lua_State* L) {
  auto s = static_cast<torchcraft::State**>(
      luaL_checkudata(L, 1, "torchcraft.State"));
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
  checkState(L);
  auto key = luaL_checkstring(L, 2);

  if (stateMembers.find(key) != stateMembers.end()) {
    return luaL_error(L, "Cannot overwrite key %s", key);
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

  lua_getuservalue(L, 1);
  lua_pushinteger(L, -1);
  lua_setfield(L, -2, "__numUpdates");
  for (auto f : {"units",
                 "resources",
                 "units_myself",
                 "units_enemy",
                 "units_neutral",
                 "resources_myself"}) {
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
    pushMember(L, s, m, -2);
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

int setconsiderState(lua_State* L) {
  auto s = checkState(L);
  if (lua_gettop(L) != 2 || !lua_istable(L, 2)) {
    return luaL_error(L, "Expected table argument");
  }
  s->setOnlyConsiderTypes(torchcraft::getConsideredTypes(L));
  return 0;
}

int cloneState(lua_State* L) {
  auto s = checkState(L);
  pushState(L, s, true);

  // Deep-copy uservalues
  lua_getuservalue(L, -1); // of new state
  lua_getuservalue(L, -3); // of old state
  torchcraft::lua::deepCopyTable(L);
  lua_pop(L, 2);
  return 1;
}

// index represents the index of the state userdata on the stack
int pushUpdatesState(
    lua_State* L,
    std::vector<std::string>& updates,
    int index) {
  lua_pushvalue(L, index);
  auto s = checkState(L, -1);
  lua_newtable(L);
  for (auto u : updates) {
    pushMember(L, s, u, -2);
    lua_setfield(L, -2, u.c_str());
  }
  lua_remove(L, -2); // remove state copy
  return 1;
}

namespace torchcraft {

std::set<BW::UnitType> getConsideredTypes(lua_State* L, int index) {
  if (!lua_istable(L, index)) {
    luaL_error(L, "getConsideredTypes: table expected");
    // does not return
  }

  std::set<torchcraft::BW::UnitType> types;
  lua_pushvalue(L, index);
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    auto ut = torchcraft::BW::UnitType::_from_integral_nothrow(
        luaL_checkinteger(L, -1));
    if (!ut) {
      luaL_error(L, "Invalid unit type: %d", lua_tointeger(L, -1));
    }
    types.emplace(*ut);
    lua_pop(L, 1);
  }
  return types;
}

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
