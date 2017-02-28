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
#include <lua.h>
#include <luaT.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <cassert>

#include "replayer.h"

torchcraft::replayer::Replayer* checkReplayer(lua_State* L, int id = 1);

extern "C" int newReplayer(lua_State* L);
extern "C" int gcReplayer(lua_State* L);
extern "C" int loadReplayer(lua_State* L);
extern "C" int replayerSave(lua_State* L);
extern "C" int replayerGetNumFrames(lua_State* L);
extern "C" int replayerGetFrame(lua_State* L);
extern "C" int replayerSetNumUnits(lua_State* L);
extern "C" int replayerSetMap(lua_State* L);
extern "C" int replayerGetMap(lua_State* L);
extern "C" int replayerPush(lua_State* L);
extern "C" int replayerGetNumUnits(lua_State* L);
extern "C" int replayerSetNumUnits(lua_State* L);


//const struct luaL_Reg replayer_m [] = {
const struct luaL_Reg replayer_m [] = {
  {"__gc", gcReplayer},
  {"save", replayerSave},
  {"getNumFrames", replayerGetNumFrames},
  {"getFrame", replayerGetFrame},
  {"setNumUnits", replayerSetNumUnits},
  {"getNumUnits", replayerGetNumUnits},
  {"setMap", replayerSetMap},
  {"getMap", replayerGetMap},
  {"push", replayerPush},
  {nullptr, nullptr}
};
