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

namespace {

// client userdata is at index, update is at top of stack
void updateState(lua_State* L, int index) {
  // push state table onto stack
  lua_getuservalue(L, index);
  lua_getfield(L, -1, "state");
  // push update onto stack
  lua_pushvalue(L, -3);
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    lua_pushvalue(L, -2);
    lua_insert(L, -2);
    if (!strcmp(lua_tostring(L, -2), "frame")) {
      lua_pushvalue(L, -1);
      lua_setfield(L, -5, "frame_string");

      replayer::Frame** f =
          (replayer::Frame**)lua_newuserdata(L, sizeof(replayer::Frame*));
      *f = new replayer::Frame();
      std::istringstream ss(lua_tostring(L, -2));
      ss >> (**f);

      luaL_getmetatable(L, "torchcraft.Frame");
      lua_setmetatable(L, -2);
      lua_remove(L, -2);
    }
    lua_settable(L, -5);
  }
  lua_pop(L, 3);
}

// client userdata is at index, value is at top of stack
void setState(lua_State* L, int index, const char* key) {
  // push state table onto stack
  lua_getuservalue(L, index);
  lua_getfield(L, -1, "state");
  // push value onto stack
  lua_pushvalue(L, -3);
  lua_setfield(L, -2, key);
  lua_pop(L, 2);
}

THByteTensor* imageToTensor(const std::string& data) {
  std::istringstream ss(data);
  std::string t;
  std::getline(ss, t, ',');
  auto width = std::stoi(t);
  std::getline(ss, t, ',');
  auto height = std::stoi(t);
  auto imgdata = data.substr(ss.tellg());

  auto storage = THByteStorage_newWithSize(3 * height * width);
  auto rgb = storage->data;

  // Incoming binary data is [BGRA,...], which we transform into [R..,G..,B..].
  int k = 0;
  for (int a = 2; a >= 0; --a) {
    int it = a;
    for (int i = 0; i < height * width; i++) {
      rgb[k] = imgdata[it];
      it += 4;
      ++k;
    }
  }

  return THByteTensor_newWithStorage3d(
      storage, 0, 3, height * width, width, height, height, 1);
}

} // namespace

int newClient(lua_State* L) {
  auto hostname = luaL_checkstring(L, 1);
  auto port = luaL_checkint(L, 2);
  client::Client* cl;
  try {
    client::Connection conn(client::Connection(hostname, port));
    cl = new client::Client(std::move(conn));
  } catch (zmq::error_t& e) {
    return luaL_error(L, e.what());
  }
  luaT_pushudata(L, cl, "torchcraft.Client");

  // uservalue contains a few raw Lua values
  lua_newtable(L);
  lua_newtable(L);
  lua_setfield(L, -2, "state");
  lua_setuservalue(L, -2);

  return 1;
}

int freeClient(lua_State* L) {
  auto cl =
      static_cast<client::Client*>(luaT_checkudata(L, 1, "torchcraft.Client"));
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
  auto cl =
      static_cast<client::Client*>(luaT_checkudata(L, 1, "torchcraft.Client"));
  auto key = luaL_checkstring(L, 2);

  luaL_getmetafield(L, 1, key);
  if (!lua_isnil(L, -1) && lua_iscfunction(L, -1)) {
    return 1;
  }
  lua_pop(L, 1);

  lua_getuservalue(L, 1);
  lua_getfield(L, -1, key);
  lua_remove(L, -2);
  return 1;
}

int initClient(lua_State* L) {
  auto cl =
      static_cast<client::Client*>(luaT_checkudata(L, 1, "torchcraft.Client"));
  client::Client::Options opts;
  if (lua_gettop(L) > 1) {
    if (!lua_istable(L, 2)) {
      return luaL_error(L, "table argument expected");
    }

    lua_getfield(L, 2, "initial_map");
    if (!lua_isnil(L, -1)) {
      opts.initialMap = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "window_size");
    if (!lua_isnil(L, -1)) {
      lua_rawgeti(L, -1, 1);
      opts.windowSize[0] = lua_toboolean(L, -1);
      lua_rawgeti(L, -2, 2);
      opts.windowSize[1] = lua_toboolean(L, -1);
      lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "window_pos");
    if (!lua_isnil(L, -1)) {
      lua_rawgeti(L, -1, 1);
      opts.windowPos[0] = lua_toboolean(L, -1);
      lua_rawgeti(L, -2, 2);
      opts.windowPos[1] = lua_toboolean(L, -1);
      lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "micro_battles");
    if (!lua_isnil(L, -1)) {
      opts.microBattles = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
  }

  std::string reply;
  if (!cl->init(reply, std::move(opts))) {
    return luaL_error(
        L, "initial connection setup failed: %s", cl->error().c_str());
  }

  luaL_dostring(L, ("return " + reply).c_str());
  updateState(L, 1);
  return 1;
}

int sendClient(lua_State* L) {
  auto cl =
      static_cast<client::Client*>(luaT_checkudata(L, 1, "torchcraft.Client"));
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
  auto cl =
      static_cast<client::Client*>(luaT_checkudata(L, 1, "torchcraft.Client"));
  std::string reply;
  if (!cl->receive(reply)) {
    return luaL_error(L, "receive failed: %s", cl->error().c_str());
  }

  // Detect optional image in reply from server
  const char marker[] = "TCIMAGEDATA";
  auto pos = reply.find(marker);
  if (pos != std::string::npos) {
    auto off = pos + sizeof(marker) - 1;
    auto image = reply.substr(off, reply.length() - off - 2);
    auto thImage = imageToTensor(image);
    luaT_pushudata(L, static_cast<void*>(thImage), "torch.ByteTensor");
    setState(L, 1, "image");

    reply = reply.substr(0, pos) + "}";
  }

  luaL_dostring(L, ("return " + reply).c_str());
  updateState(L, 1);
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
