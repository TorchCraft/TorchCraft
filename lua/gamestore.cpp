/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "gamestore.h"

using namespace std;
using namespace replayer;

// Serialization

std::ostream& replayer::operator<<(std::ostream& out,
    const replayer::CircularBuffer& o) {
  out << o.size << " " << o.head << " " << o.tail << " " << o.length;
  for(size_t i = o.tail + 1; i <= o.tail + o.length; i++) {
    out << " " << *(o.buffer[i % o.size]);
  }
  return out;
}

std::istream& replayer::operator>>(std::istream& in,
    replayer::CircularBuffer& o) {
  in >> o.size >> o.head >> o.tail >> o.length;
  o.buffer.resize(o.size);
  for(size_t i = 0; i < o.length; i++) {
    Replayer* r = new Replayer();
    in >> *r;
    o.buffer[(o.tail + 1 + i) % o.size] = r;
  }
  return in;
}

std::ostream& replayer::operator<<(std::ostream& out, const GameStore& gs) {
  out << gs.lost << " " << gs.won;
  return out;
}

std::istream& replayer::operator>>(std::istream& in, GameStore& gs) {
  in >> gs.lost >> gs.won;
  std::cout << "Loaded Gamestore : "
    << " lost.getSize() = " << gs.lost.getSize()
    << "  won.getSize() = " << gs.won.getSize() << std::endl;
  return in;
}

// Utility

GameStore* checkGameStore(lua_State* L, int id) {
  void *gs = luaL_checkudata(L, id, "torchcraft.GameStore");
  luaL_argcheck(L, gs != nullptr, id, "'GameStore' expected");
  return *(GameStore**) gs;
}

// Constructors/Destructors/Load/Save

extern "C" int newGameStore(lua_State* L) {
  auto lost = luaL_checkint(L, 1);
  auto won = luaL_checkint(L, 2);

  auto gs = (GameStore **)lua_newuserdata(L, sizeof(GameStore *));
  *gs = new GameStore(lost, won);

  luaL_getmetatable(L, "torchcraft.GameStore");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int gcGameStore(lua_State* L){
  auto gs = (GameStore**)luaL_checkudata(L, 1, "torchcraft.GameStore");

  assert(*gs != nullptr);
  delete *gs;
  *gs = nullptr;

  return 0;
}

extern "C" int loadGameStore(lua_State* L) {
  auto file = luaL_checkstring(L, 1);

  auto gs = (GameStore **)lua_newuserdata(L, sizeof(GameStore *));
  *gs = new GameStore();

  std::ifstream in(file);
  in >> **gs;

  luaL_getmetatable(L, "torchcraft.GameStore");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int gameStoreSave(lua_State* L) {
  auto gs = checkGameStore(L);
  auto file = luaL_checkstring(L, 2);

  std::ofstream out(file);
  out << *gs;

  return 0;
}

// Accessors & other methods

extern "C" int gameStoreAdd(lua_State* L) {
  auto gs = checkGameStore(L, 1);
  auto r = checkReplayer(L, 2);
  luaL_argcheck(L, lua_isboolean(L, 3), 3, "expected boolean");
  gs->add(r, lua_toboolean(L, 3));
  return 0;
}

extern "C" int gameStoreSample(lua_State* L) {
  auto gs = checkGameStore(L, 1);
  luaL_argcheck(L, gs->getTotalSize(), 1, "GameStore is empty");
  auto prop_won = luaL_checknumber(L, 2);

  auto r = (Replayer **)lua_newuserdata(L, sizeof(Replayer *));
  *r = gs->sample(prop_won);
  assert(*r != nullptr);
  (*r)->incref();

  luaL_getmetatable(L, "torchcraft.Replayer");
  lua_setmetatable(L, -2);
  return 1;
}

extern "C" int gameStoreGetSizeLost(lua_State* L) {
  auto gs = checkGameStore(L, 1);
  luaL_argcheck(L, gs->getTotalSize(), 1, "GameStore is empty");
  lua_pushnumber(L, gs->getSizeLost());
  return 1;
}

extern "C" int gameStoreGetLastBattlesLost(lua_State* L) {
  auto gs = checkGameStore(L, 1);
  luaL_argcheck(L, gs->getTotalSize(), 1, "GameStore is empty");
  auto nbattles = luaL_checknumber(L, 2);
  lua_newtable(L);
  for (int i = 0; i < nbattles; ++i)
  {
    lua_pushnumber(L, i+1);
    auto r = (Replayer **)lua_newuserdata(L, sizeof(Replayer *));

    *r = gs->getLastLost(i);
    assert(*r != nullptr);
    (*r)->incref();

    luaL_getmetatable(L, "torchcraft.Replayer");
    lua_setmetatable(L, -2);
    lua_settable(L, -3);
  }
  return 1;
}

extern "C" int gameStoreGetLast(lua_State* L) {
  auto gs = checkGameStore(L);
  luaL_argcheck(L, gs->getTotalSize(), 1, "GameStore is empty");
  auto r = (Replayer **)lua_newuserdata(L, sizeof(Replayer *));

  *r = gs->getLast();
  assert(*r != nullptr);
  (*r)->incref();

  luaL_getmetatable(L, "torchcraft.Replayer");
  lua_setmetatable(L, -2);
  return 1;
}

