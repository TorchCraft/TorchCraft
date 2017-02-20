/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "replayer.h"

#include "frame_lua.h"

using namespace std;
using namespace replayer;

// Serialization

ostream& replayer::operator<<(ostream& out, const replayer::Replayer& o) {
  auto height = THByteTensor_size(o.map.data, 0);
  auto width = THByteTensor_size(o.map.data, 1);
  auto data = THByteTensor_data(o.map.data);

  out << height << " " << width << " ";
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      out << data[y * width + x] << " ";
    }
  }

  out << o.frames.size() << " ";
  for (auto& f : o.frames) {
    out << *f << " ";
  }

  out<< o.numUnits.size() << " ";
  for (const auto& nu : o.numUnits) {
    out << nu.first << " " << nu.second << " ";
  }

  return out;
}

istream& replayer::operator>>(istream& in, replayer::Replayer& o) {
  // WARNING: cases were observed where this operator left a Replayer
  // that was in a corrupted state, and would produce a segfault
  // if we tried to delete it.
  // Cause: invalid data file? I/O error? or a bug in the code?

  int height, width;
  in >> height >> width;
  if (height <= 0 || width <= 0)
    throw std::runtime_error("Corrupted replay: invalid map size");
  uint8_t* data = (uint8_t*) THAlloc(sizeof(uint8_t) * height * width);
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      in >> data[y * width + x];
    }
  }
  o.setMap(height, width, data);
  int nFrames;
  in >> nFrames;
  if (nFrames < 0) throw std::runtime_error("Corrupted replay: nFrames < 0");
  o.frames.resize(nFrames);
  for (size_t i = 0; i < nFrames; i++) {
    o.frames[i] = new Frame();
    in >> *o.frames[i];
  }

  int s;
  in >> s;
  if (s < 0) throw std::runtime_error("Corrupted replay: s < 0");
  int32_t key, val;
  for (auto i = 0; i < s; i++) {
    in >> key >> val;
    o.numUnits[key] = val;
  }

  return in;
}

// Utility

replayer::Replayer* checkReplayer(lua_State* L, int id) {
  void *r = luaL_checkudata(L, id, "torchcraft.Replayer");
  luaL_argcheck(L, r != nullptr, id, "'replayer' expected");
  return *(replayer::Replayer**) r;
}

// Constructor/Destructor/Load/Save

extern "C" int newReplayer(lua_State* L) {
  Replayer **r = (Replayer **)lua_newuserdata(L, sizeof(Replayer *));
  *r = new Replayer();

  luaL_getmetatable(L, "torchcraft.Replayer");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int gcReplayer(lua_State* L) {
  auto r = (replayer::Replayer**)luaL_checkudata(L, 1, "torchcraft.Replayer");

  assert(*r != nullptr);
  (*r)->decref();
  *r = nullptr;

  return 0;
}

extern "C" int loadReplayer(lua_State* L) {
  auto path = luaL_checkstring(L, 1);
  std::ifstream in(path);
  luaL_argcheck(L, in, 1, "Invalid load path");

  Replayer *rep = nullptr;
  try {
    rep = new Replayer();
    in >> *rep;
  } catch (std::exception& e) {
    in.close();
    if (rep != nullptr) delete rep;

    luaL_error(L, "C++ exception in loadReplayer: %s", e.what());
    assert(false);
  }
  in.close();

  Replayer **r = (Replayer **)lua_newuserdata(L, sizeof(Replayer *));
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
  auto f = (replayer::Frame **)lua_newuserdata(L, sizeof(replayer::Frame *));
  size_t id = luaL_checkint(L, 2) - 1;
  luaL_argcheck(L, id >= 0 && id < r->size(), 2, "invalid index");

  *f = r->getFrame(id);
  (*f)->incref();

  luaL_getmetatable(L, "torchcraft.Frame");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int replayerGetNumUnits(lua_State* L) {
  auto r  = checkReplayer(L);
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
      luaT_checkudata(L, 2, "torch.ByteTensor")
      );
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
  lua_pushnumber(L, (lua_Number) r->size());
  return 1;
}

extern "C" int replayerPush(lua_State* L) {
  auto r = checkReplayer(L);
  auto f = checkFrame(L, 2);
  r->push(f);
  return 0;
}
