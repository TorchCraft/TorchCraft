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

#include <TH/TH.h>
#include <lauxlib.h>
#include <lua.h>
#include <luaT.h>
#include <lualib.h>

int newState(lua_State* L);
int pushState(lua_State* L, client::State* s = nullptr);
int freeState(lua_State* L);
int gcState(lua_State* L);
int indexState(lua_State* L);
int newindexState(lua_State* L);
int resetState(lua_State* L);
int totableState(lua_State* L);
int pushUpdatesState(
    lua_State* L,
    std::vector<std::string>& updates,
    int index = -1);

const struct luaL_Reg state_m[] = {
    {"__gc", gcState},
    {"__index", indexState},
    {"__newindex", newindexState},
    {"reset", resetState},
    {"toTable", totableState},
    {nullptr, nullptr},
};

} // extern "C"

namespace client {
void registerState(lua_State* L, int index);
}
