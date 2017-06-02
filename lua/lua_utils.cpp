/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

extern "C" {
#include <TH/TH.h>
}

#include "lua_utils.h"

namespace torchcraft {
namespace lua {

int sealedTableGuard(lua_State* L) {
  return luaL_error(L, "Attempting to add a field to a sealed table");
}

// Prevents adding new fields to the table at the given index
void sealTable(lua_State* L, int index) {
  lua_pushvalue(L, index);
  lua_newtable(L);
  lua_pushcfunction(L, sealedTableGuard);
  lua_setfield(L, -2, "__newindex");
  lua_setmetatable(L, -2);
  lua_pop(L, 1);
}

// Deep-copy a table into a new one
void deepCloneTable(lua_State* L, int index) {
  lua_pushvalue(L, index);
  lua_newtable(L);
  lua_insert(L, -2);
  deepCopyTable(L);
  lua_pop(L, 1);
}


// Deep-copy a table into a given one
void deepCopyTable(lua_State* L, int dindex, int index) {
  lua_pushvalue(L, dindex);
  lua_pushvalue(L, index < 0 ? index-1 : index);

  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    // Push key for insertion
    lua_pushvalue(L, -2);
    lua_insert(L, -2);

    // value at -1: push copy onto stack
    switch (lua_type(L, -1)) {
      case LUA_TNIL:
        lua_pushnil(L);
        break;
      case LUA_TBOOLEAN:
        lua_pushboolean(L, lua_toboolean(L, -1));
        break;
      case LUA_TNUMBER:
        lua_pushnumber(L, lua_tonumber(L, -1));
        break;
      case LUA_TSTRING:
        lua_pushlstring(L, lua_tostring(L, -1), lua_strlen(L, -1));
        break;
      case LUA_TTABLE:
        deepCloneTable(L);
        break;
      default:
        lua_pushvalue(L, -1); // unknown type: shallow copy
        break;
    }

    // remove original value
    lua_remove(L, -2);
    lua_settable(L, -5);
  }

  lua_pop(L, 2);
}

} // namespace lua
} // namespace torchcraft
