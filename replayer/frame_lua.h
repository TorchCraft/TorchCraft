/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

extern "C" {
#include <TH/TH.h>
#include <lua.h>
#include <luaT.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <cassert>

#include "frame.h"

replayer::Frame* checkFrame(lua_State* L, int id = 1);

extern "C" int frameFromTable(lua_State* L);
extern "C" int frameFromString(lua_State* L);
extern "C" int frameClone(lua_State* L);
extern "C" int frameCombine(lua_State* L);
extern "C" int frameToString(lua_State* L);
extern "C" int frameToTable(lua_State* L);
extern "C" int frameGetUnits(lua_State* L);
extern "C" int frameGetResources(lua_State* L);
extern "C" int frameGetNumPlayers(lua_State* L);
extern "C" int frameGetNumUnits(lua_State* L);
extern "C" int gcFrame(lua_State* L);

// a bunch of utilities to manipulate the stack
void setInt(lua_State* L, const char* key, int v);
void setBool(lua_State* L, const char* key, bool v);
void getField(lua_State* L, const char* key);
int getInt(lua_State* L, const char* key);
bool getBool(lua_State* L, const char* key);

// Lua tables from/to Frame class
void toFrame(lua_State* L, int id, replayer::Frame& res);
void pushFrame(lua_State* L, const replayer::Frame& res);

const struct luaL_Reg frame_m [] = {
  {"__gc", gcFrame},
  {"clone", frameClone},
  {"combine", frameCombine},
  {"toTable", frameToTable},
  {"toString", frameToString},
  {"getUnits", frameGetUnits},
  {"getResources", frameGetResources},
  {"getNumPlayers", frameGetNumPlayers},
  {"getNumUnits", frameGetNumUnits},
  {nullptr, nullptr}
};
