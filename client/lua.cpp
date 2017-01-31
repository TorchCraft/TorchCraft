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
#include <lauxlib.h>
#include <lua.h>
#include <luaT.h>
#include <lualib.h>
}

#include "client_lua.h"
#include "state_lua.h"

extern "C" int luaopen_torchcraft_client(lua_State* L) {
  lua_newtable(L);
  client::registerClient(L, lua_gettop(L));
  client::registerState(L, lua_gettop(L));
  return 1;
}
