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
#include <lauxlib.h>
#include <lua.h>
#include <luaT.h>
#include <lualib.h>

int newClient(lua_State* L);
int freeClient(lua_State* L);
int gcClient(lua_State* L);
int indexClient(lua_State* L);
int connectClient(lua_State* L);
int connectedClient(lua_State* L);
int closeClient(lua_State* L);
int initClient(lua_State* L);
int sendClient(lua_State* L);
int receiveClient(lua_State* L);

const struct luaL_Reg client_m[] = {
    {"__gc", gcClient},
    {"__index", indexClient},
    {"connect", connectClient},
    {"connected", connectedClient},
    {"close", closeClient},
    {"init", initClient},
    {"send", sendClient},
    {"receive", receiveClient},
    {nullptr, nullptr},
};

} // extern "C"

namespace client {
void registerClient(lua_State* L, int index);
}
