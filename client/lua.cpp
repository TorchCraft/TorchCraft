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
#include "client.h"
#include "client_lua.h"
#include "constants_lua.h"
#include "state_lua.h"

namespace client {
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

} // namespace lua
} // namespace client

extern "C" int luaopen_torchcraft_client(lua_State* L) {
  client::init();

  lua_newtable(L);
  client::registerClient(L, lua_gettop(L));
  client::registerState(L, lua_gettop(L));
  client::registerConstants(L, lua_gettop(L));
  return 1;
}
