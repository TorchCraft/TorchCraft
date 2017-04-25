/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "state.h"

extern "C" {

#include <lauxlib.h>
#include <lua.h>

int newState(lua_State* L);
int pushState(lua_State* L, torchcraft::State* s = nullptr);
int pushUpdatesState(
    lua_State* L,
    std::vector<std::string>& updates,
    int index = -1);
int freeState(lua_State* L);
int gcState(lua_State* L);
int indexState(lua_State* L);
int newindexState(lua_State* L);
int resetState(lua_State* L);
int totableState(lua_State* L);
int setconsiderState(lua_State* L);

const struct luaL_Reg state_m[] = {
    {"__gc", gcState},
    {"__index", indexState},
    {"__newindex", newindexState},
    {"reset", resetState},
    {"toTable", totableState},
    {"setOnlyConsiderTypes", setconsiderState},
    {nullptr, nullptr},
};

} // extern "C"

namespace torchcraft {
std::set<torchcraft::BW::UnitType> getConsideredTypes(
    lua_State* L,
    int index = -1);

void registerState(lua_State* L, int index);
}
