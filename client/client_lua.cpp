/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>
#include <sstream>

#include "client.h"
#include "client_lua.h"
#include "replayer/frame_lua.h"
#include "state_lua.h"

namespace {

inline client::Client* checkClient(lua_State* L, int index = 1) {
  auto s = luaL_checkudata(L, index, "torchcraft.Client");
  luaL_argcheck(L, s != nullptr, index, "'client' expected");
  return *static_cast<client::Client**>(s);
}

} // namespace

int newClient(lua_State* L) {
  client::Client* cl = new client::Client(L);
  luaT_pushudata(L, cl, "torchcraft.Client");

  // Store Lua wrapped state in uservalue table so that all changes done by Lua
  // code are persistent.
  lua_newtable(L);
  pushState(L, cl->state());
  lua_setfield(L, -2, "state");
  lua_setuservalue(L, -2);
  return 1;
}

int freeClient(lua_State* L) {
  auto cl = checkClient(L);
  delete cl;
  return 0;
}

int gcClient(lua_State* L) {
  auto cl =
      static_cast<client::Client**>(luaL_checkudata(L, 1, "torchcraft.Client"));
  assert(*cl != nullptr);
  delete *cl;
  *cl = nullptr;
  return 0;
}

int indexClient(lua_State* L) {
  auto cl = checkClient(L);
  auto key = luaL_checkstring(L, 2);

  if (luaL_getmetafield(L, 1, key)) {
    if (!lua_isnil(L, -1) && lua_iscfunction(L, -1)) {
      return 1;
    }
    lua_pop(L, 1);
  }

  lua_getuservalue(L, 1);
  lua_getfield(L, -1, key);
  lua_remove(L, -2);
  return 1;
}

int connectClient(lua_State* L) {
  auto cl = checkClient(L);
  auto hostname = luaL_checkstring(L, 2);
  auto port = luaL_checkint(L, 3);
  if (!cl->connect(hostname, port)) {
    auto err = "connect failed: " + cl->error();
    return luaL_error(L, err.c_str());
  }
  return 0;
}

int connectedClient(lua_State* L) {
  auto cl = checkClient(L);
  lua_pushboolean(L, cl->connected());
  return 1;
}

int closeClient(lua_State* L) {
  auto cl = checkClient(L);
  if (!cl->close()) {
    auto err = "close failed: " + cl->error();
    return luaL_error(L, err.c_str());
  }
  return 0;
}

int initClient(lua_State* L) {
  auto cl = checkClient(L);
  client::Client::Options opts;
  if (lua_gettop(L) > 1) {
    if (!lua_istable(L, 2)) {
      return luaL_error(L, "table argument expected");
    }

    lua_getfield(L, 2, "initial_map");
    if (!lua_isnil(L, -1)) {
      opts.initial_map = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "window_size");
    if (!lua_isnil(L, -1)) {
      lua_rawgeti(L, -1, 1);
      opts.window_size[0] = lua_toboolean(L, -1);
      lua_rawgeti(L, -2, 2);
      opts.window_size[1] = lua_toboolean(L, -1);
      lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "window_pos");
    if (!lua_isnil(L, -1)) {
      lua_rawgeti(L, -1, 1);
      opts.window_pos[0] = lua_toboolean(L, -1);
      lua_rawgeti(L, -2, 2);
      opts.window_pos[1] = lua_toboolean(L, -1);
      lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "micro_battles");
    if (!lua_isnil(L, -1)) {
      opts.micro_battles = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
  }

  std::string reply;
  if (!cl->init(reply, std::move(opts))) {
    return luaL_error(
        L, "initial connection setup failed: %s", cl->error().c_str());
  }

  luaL_dostring(L, ("return " + reply).c_str());
  return 1;
}

int sendClient(lua_State* L) {
  auto cl = checkClient(L);
  std::string msg;
  if (lua_istable(L, 2)) {
    lua_pushvalue(L, 2);
    std::ostringstream ss;
    lua_pushnil(L);
    int index = 1;
    bool first = true;
    while (lua_next(L, -2) != 0) {
      const char* item = lua_tostring(L, -1);
      lua_pop(L, 1);
      if (!first) {
        ss << ":" << item;
      } else {
        ss << item;
        first = false;
      }
    }
    lua_pop(L, 1);
    msg = ss.str();
  } else {
    msg = luaL_checkstring(L, 2);
  }

  if (!cl->send(msg)) {
    return luaL_error(L, "send failed: %s", cl->error().c_str());
  }
  return 0;
}

int receiveClient(lua_State* L) {
  auto cl = checkClient(L);
  std::string reply;
  if (!cl->receive(reply)) {
    return luaL_error(L, "receive failed: %s", cl->error().c_str());
  }

  luaL_dostring(L, ("return " + reply).c_str());
  return 1;
}

namespace client {
void registerClient(lua_State* L, int index) {
  luaT_newlocalmetatable(
      L,
      "torchcraft.Client",
      nullptr,
      ::newClient,
      ::freeClient,
      nullptr,
      index);
  luaL_newmetatable(L, "torchcraft.Client");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaT_setfuncs(L, ::client_m, 0);
  lua_setfield(L, -2, "Client");
  lua_pop(L, 1);
}
}
