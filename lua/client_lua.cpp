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
#include "lua_utils.h"
#include "state_lua.h"

#include "frame_lua.h"

namespace {

inline client::Client* checkClient(lua_State* L, int index = 1) {
  auto s = luaL_checkudata(L, index, "torchcraft.Client");
  luaL_argcheck(L, s != nullptr, index, "'client' expected");
  return *static_cast<client::Client**>(s);
}

client::Client::Command parseCommand(const std::string& str) {
  client::Client::Command comm;
  bool gotCode = false;
  std::istringstream ss(str);
  for (std::string arg; std::getline(ss, arg, ',');) {
    if (!gotCode) {
      comm.code = std::stoi(arg);
      gotCode = true;
    } else {
      try {
        comm.args.push_back(std::stoi(arg));
      } catch (std::invalid_argument& e) {
        comm.str = arg;
      }
    }
  }
  return comm;
}

std::vector<client::Client::Command> parseCommandString(
    const std::string& str) {
  std::vector<client::Client::Command> comms;
  std::istringstream ss(str);
  for (std::string part; std::getline(ss, part, ':');) {
    comms.emplace_back(parseCommand(part));
  }
  return comms;
}

} // namespace

int newClient(lua_State* L) {
  client::Client* cl = new client::Client();
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
  checkClient(L);
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
  int nargs = lua_gettop(L);
  int timeoutMs = (nargs > 3 ? luaL_checkint(L, 4) : -1);
  if (!cl->connect(hostname, port, timeoutMs)) {
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

    lua_getfield(L, 2, "only_consider_types");
    if (!lua_isnil(L, -1)) {
      opts.only_consider_types = client::getConsideredTypes(L);
    }
    lua_pop(L, 1);
  }

  std::vector<std::string> updates;
  if (!cl->init(updates, opts)) {
    auto err = "initial connection setup failed: " + cl->error();
    return luaL_error(L, err.c_str());
  }

  lua_getuservalue(L, 1);
  lua_getfield(L, -1, "state");
  lua_remove(L, -2);
  pushUpdatesState(L, updates);
  lua_remove(L, -2);
  return 1;
}

int sendClient(lua_State* L) {
  auto cl = checkClient(L);
  std::vector<client::Client::Command> comms;

  if (lua_istable(L, 2)) {
    lua_pushvalue(L, 2);
    std::ostringstream ss;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      auto cs = parseCommandString(luaL_checkstring(L, -1));
      std::move(cs.begin(), cs.end(), std::back_inserter(comms));
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  } else {
    comms = parseCommandString(luaL_checkstring(L, 2));
  }

  if (!cl->send(comms)) {
    auto err = "send failed: " + cl->error();
    return luaL_error(L, err.c_str());
  }
  return 0;
}

int receiveClient(lua_State* L) {
  auto cl = checkClient(L);
  std::vector<std::string> updates;
  if (!cl->receive(updates)) {
    auto err = "receive failed: " + cl->error();
    return luaL_error(L, err.c_str());
  }

  lua_getuservalue(L, 1);
  lua_getfield(L, -1, "state");
  lua_remove(L, -2);
  pushUpdatesState(L, updates);
  lua_remove(L, -2);
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
