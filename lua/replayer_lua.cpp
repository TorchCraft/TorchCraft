/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifdef WITH_ZSTD
#include "zstdstream.h"
#endif

#include "replayer_lua.h"
#include "state_lua.h"

#include "frame_lua.h"

using namespace std;
using namespace torchcraft::replayer;

extern "C" int newReplayer(lua_State* L) {
  Replayer** r = (Replayer**)lua_newuserdata(L, sizeof(Replayer*));
  *r = new Replayer();

  luaL_getmetatable(L, "torchcraft.Replayer");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int gcReplayer(lua_State* L) {
  auto r = (Replayer**)luaL_checkudata(L, 1, "torchcraft.Replayer");

  assert(*r != nullptr);
  (*r)->decref();
  *r = nullptr;

  return 0;
}

extern "C" int loadReplayer(lua_State* L) {
  auto path = luaL_checkstring(L, 1);
#ifdef WITH_ZSTD
  // zstd::ifstream will auto-detect compressed data
  zstd::ifstream in(path);
#else
  std::ifstream in(path);
#endif
  luaL_argcheck(L, in, 1, "Invalid load path");

  Replayer* rep = nullptr;
  try {
    rep = new Replayer();
    in >> *rep;
  } catch (std::exception& e) {
    in.close();
    if (rep != nullptr)
      delete rep;

    luaL_error(L, "C++ exception in loadReplayer: %s", e.what());
    assert(false);
  }
  in.close();

  Replayer** r = (Replayer**)lua_newuserdata(L, sizeof(Replayer*));
  *r = rep;
  luaL_getmetatable(L, "torchcraft.Replayer");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int replayerSave(lua_State* L) {
  auto r = checkReplayer(L);
  auto path = luaL_checkstring(L, 2);
  bool compressed = false;
  if (lua_gettop(L) > 2) {
    compressed = lua_toboolean(L, 3);
  }
#ifndef WITH_ZSTD
  if (compressed) {
    std::cerr << "Warning: no Zstd support; disabling "
              << "compression for saved replay" << std::endl;
    compressed = false;
  }
#endif

  if (compressed) {
#ifdef WITH_ZSTD
    zstd::ofstream out(path);
    luaL_argcheck(L, out, 2, "invalid save path");
    out << *r;
    out.close();
#endif
  } else {
    std::ofstream out(path);
    luaL_argcheck(L, out, 2, "invalid save path");
    out << *r;
    out.close();
  }
  return 0;
}

// Accessors & other methods

extern "C" int replayerGetFrame(lua_State* L) {
  auto r = checkReplayer(L);
  auto f = (Frame**)lua_newuserdata(L, sizeof(Frame*));
  size_t id = luaL_checkint(L, 2) - 1;
  luaL_argcheck(L, id >= 0 && id < r->size(), 2, "invalid index");

  *f = r->getFrame(id);
  (*f)->incref();

  luaL_getmetatable(L, "torchcraft.Frame");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int replayerSetKeyFrame(lua_State* L) {
  auto r = checkReplayer(L);
  auto kf = luaL_checkint(L, 2);
  r->setKeyFrame(kf);
  return 0;
}

extern "C" int replayerGetKeyFrame(lua_State* L) {
  auto r = checkReplayer(L);
  auto n = r->getKeyFrame();
  lua_pushnumber(L, n);
  return 1;
}

extern "C" int replayerGetNumUnits(lua_State* L) {
  auto r = checkReplayer(L);
  auto id = luaL_checkint(L, 2);
  auto n = r->getNumUnits(id);
  luaL_argcheck(L, n >= 0, 2, "invalid index");
  lua_pushnumber(L, n);
  return 1;
}

extern "C" int replayerSetNumUnits(lua_State* L) {
  auto r = checkReplayer(L);
  r->setNumUnits();
  return 0;
}

extern "C" int replayerSetMap(lua_State* L) {
  // arguments are (walkability, ground_height, buildability, start_locs)
  auto r = checkReplayer(L);
  std::vector<int> start_loc_x, start_loc_y;

  auto s = luaL_checkudata(L, 2, "torchcraft.State");
  if (s != nullptr) {
    torchcraft::State* state = *static_cast<torchcraft::State**>(s);
    r->setMapFromState(state);
    return 0;
  }

  THByteTensor* walkmap = reinterpret_cast<THByteTensor*>(
      luaT_checkudata(L, 2, "torch.ByteTensor"));
  THByteTensor* heightmap = reinterpret_cast<THByteTensor*>(
      luaT_checkudata(L, 3, "torch.ByteTensor"));
  THByteTensor* buildmap = reinterpret_cast<THByteTensor*>(
      luaT_checkudata(L, 4, "torch.ByteTensor"));
  if (!lua_istable(L, 5))
    luaL_error(L, "bad argument #%d (argument must be table)", 5);
  while (lua_next(L, 5) != 0) {
    if (!lua_istable(L, -1))
      luaL_error(
          L,
          "start location element %d should be a table",
          luaL_checkint(L, -2));
    luaT_getfieldcheckint(L, -1, "x");
    luaT_getfieldcheckint(L, -2, "y");
    start_loc_x.push_back(luaL_checkint(L, -2));
    start_loc_y.push_back(luaL_checkint(L, -1));
    lua_pop(L, 3);
  }
  auto h = THByteTensor_size(walkmap, 0);
  auto w = THByteTensor_size(walkmap, 0);
  walkmap = THByteTensor_newContiguous(walkmap);
  heightmap = THByteTensor_newContiguous(heightmap);
  buildmap = THByteTensor_newContiguous(buildmap);
  r->setMap(
      h,
      w,
      THByteTensor_data(walkmap),
      THByteTensor_data(heightmap),
      THByteTensor_data(buildmap),
      start_loc_x,
      start_loc_y);
  return 0;
}

extern "C" int replayerGetMap(lua_State* L) {
  auto r = checkReplayer(L);
  THByteTensor* walkmap = THByteTensor_new();
  THByteTensor* heightmap = THByteTensor_new();
  THByteTensor* buildmap = THByteTensor_new();
  std::vector<int> start_loc_x, start_loc_y;
  std::vector<uint8_t> w_vec, h_vec, b_vec;
  auto dim = r->getMap(w_vec, h_vec, b_vec, start_loc_x, start_loc_y);
  THByteTensor_resize2d(walkmap, dim.first, dim.second);
  THByteTensor_resize2d(heightmap, dim.first, dim.second);
  THByteTensor_resize2d(buildmap, dim.first, dim.second);
  std::memcpy(THByteTensor_data(walkmap), w_vec.data(), w_vec.size());
  std::memcpy(THByteTensor_data(heightmap), h_vec.data(), h_vec.size());
  std::memcpy(THByteTensor_data(buildmap), b_vec.data(), b_vec.size());
  luaT_pushudata(L, walkmap, "torch.ByteTensor");
  luaT_pushudata(L, heightmap, "torch.ByteTensor");
  luaT_pushudata(L, buildmap, "torch.ByteTensor");
  lua_createtable(L, start_loc_x.size(), 0);
  for (int i = 0; i < start_loc_x.size(); i++) {
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, start_loc_x[i]);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, start_loc_y[i]);
    lua_setfield(L, -2, "y");
    lua_rawseti(L, -2, i + 1);
  }
  return 4;
}

extern "C" int replayerGetNumFrames(lua_State* L) {
  auto r = checkReplayer(L);
  lua_pushnumber(L, (lua_Number)r->size());
  return 1;
}

extern "C" int replayerPush(lua_State* L) {
  auto r = checkReplayer(L);
  auto f = checkFrame(L, 2);
  r->push(f);
  return 0;
}

// Utility

Replayer* checkReplayer(lua_State* L, int id) {
  void* r = luaL_checkudata(L, id, "torchcraft.Replayer");
  luaL_argcheck(L, r != nullptr, id, "'replayer' expected");
  return *(Replayer**)r;
}
