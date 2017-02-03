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
inline void pushValue(lua_State* L, const char* val) {
  lua_pushstring(L, val);
}
inline void pushValue(lua_State* L, const std::string& val) {
  lua_pushstring(L, val.c_str());
}
inline void pushValue(lua_State* L, lua_CFunction val) {
  lua_pushcfunction(L, val);
}
template <typename T>
inline void pushValue(lua_State* L, const std::vector<T>& val) {
  lua_createtable(L, val.size(), 0);
  int i = 1;
  for (const auto& e : val) {
    pushValue(L, e);
    lua_rawseti(L, -2, i++);
  }
}

template <typename T>
inline void pushToTable(lua_State* L, int index, const char* key, T val) {
  pushValue(L, val);
  lua_setfield(L, index, key);
}

} // namespace lua
} // namespace client
