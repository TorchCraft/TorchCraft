/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "replayer_lua.h"

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
  std::ifstream in(path);
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
  std::ofstream out(path);
  luaL_argcheck(L, out, 2, "invalid save path");
  out << *r;
  out.close();
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
  auto r = checkReplayer(L);
  THByteTensor* map = reinterpret_cast<THByteTensor*>(
      luaT_checkudata(L, 2, "torch.ByteTensor"));
  r->setMap(map);
  return 0;
}

extern "C" int replayerGetMap(lua_State* L) {
  auto r = checkReplayer(L);
  THByteTensor_retain(r->getMap());
  luaT_pushudata(L, r->getMap(), "torch.ByteTensor");
  return 1;
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
