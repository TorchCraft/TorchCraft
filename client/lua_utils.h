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

extern "C" {

#include <lauxlib.h>
#include <lua.h>
#include <luaT.h>
#include <lualib.h>

} // extern "C"

namespace client {
namespace lua {

void sealTable(lua_State* L, int index = -1);

inline void pushValue(lua_State* L, bool val) {
  lua_pushboolean(L, val);
}
inline void pushValue(lua_State* L, int val) {
  lua_pushinteger(L, val);
}
inline void pushValue(lua_State* L, double val) {
  lua_pushnumber(L, val);
}
inline void pushValue(lua_State* L, const std::string& val) {
  lua_pushstring(L, val.c_str());
}

} // namespace lua
} // namespace client
