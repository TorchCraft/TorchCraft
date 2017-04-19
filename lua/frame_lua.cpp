/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "frame_lua.h"

using namespace std;
using namespace torchcraft::replayer;

namespace {

std::unordered_map<uint64_t, const char*> flagNames = {
    {Unit::Flags::Accelerating, "accelerating"},
    {Unit::Flags::Attacking, "attacking"},
    {Unit::Flags::AttackFrame, "attack_frame"},
    {Unit::Flags::BeingConstructed, "being_constructed"},
    {Unit::Flags::BeingGathered, "being_gathered"},
    {Unit::Flags::BeingHealed, "being_healed"},
    {Unit::Flags::Blind, "blind"},
    {Unit::Flags::Braking, "braking"},
    {Unit::Flags::Burrowed, "burrowed"},
    {Unit::Flags::CarryingGas, "carrying_gas"},
    {Unit::Flags::CarryingMinerals, "carrying_minerals"},
    {Unit::Flags::Cloaked, "cloaked"},
    {Unit::Flags::Completed, "completed"},
    {Unit::Flags::Constructing, "constructing"},
    {Unit::Flags::DefenseMatrixed, "defense_matrixed"},
    {Unit::Flags::Detected, "detected"},
    {Unit::Flags::Ensnared, "ensnared"},
    {Unit::Flags::Flying, "flying"},
    {Unit::Flags::Following, "following"},
    {Unit::Flags::GatheringGas, "gathering_gas"},
    {Unit::Flags::GatheringMinerals, "gathering_minerals"},
    {Unit::Flags::Hallucination, "hallucination"},
    {Unit::Flags::HoldingPosition, "holding_position"},
    {Unit::Flags::Idle, "idle"},
    {Unit::Flags::Interruptible, "interruptible"},
    {Unit::Flags::Invincible, "invincible"},
    {Unit::Flags::Irradiated, "irradiated"},
    {Unit::Flags::Lifted, "lifted"},
    {Unit::Flags::Loaded, "loaded"},
    {Unit::Flags::LockedDown, "locked_down"},
    {Unit::Flags::Maelstrommed, "maelstrommed"},
    {Unit::Flags::Morphing, "morphing"},
    {Unit::Flags::Moving, "moving"},
    {Unit::Flags::Parasited, "parasited"},
    {Unit::Flags::Patrolling, "patrolling"},
    {Unit::Flags::Plagued, "plagued"},
    {Unit::Flags::Powered, "powered"},
    {Unit::Flags::Repairing, "repairing"},
    {Unit::Flags::Researching, "researching"},
    {Unit::Flags::Selected, "selected"},
    {Unit::Flags::Sieged, "sieged"},
    {Unit::Flags::StartingAttack, "starting_attack"},
    {Unit::Flags::Stasised, "stasised"},
    {Unit::Flags::Stimmed, "stimmed"},
    {Unit::Flags::Stuck, "stuck"},
    {Unit::Flags::Targetable, "targetable"},
    {Unit::Flags::Training, "training"},
    {Unit::Flags::UnderAttack, "under_attack"},
    {Unit::Flags::UnderDarkSwarm, "under_dark_swarm"},
    {Unit::Flags::UnderDisruptionWeb, "under_disruption_web"},
    {Unit::Flags::UnderStorm, "under_storm"},
    {Unit::Flags::Upgrading, "upgrading"}};

} // namespace

// Utility

Frame* checkFrame(lua_State* L, int id) {
  void* f = luaL_checkudata(L, id, "torchcraft.Frame");
  luaL_argcheck(L, f != nullptr, id, "'frame' expected");
  return *(Frame**)f;
}

// Frame construtors & destructors

extern "C" int gcFrame(lua_State* L) {
  Frame** f = (Frame**)luaL_checkudata(L, 1, "torchcraft.Frame");

  assert(*f != nullptr);
  (*f)->decref();
  *f = nullptr;

  return 0;
}

extern "C" int frameFromTable(lua_State* L) {
  Frame* f = new Frame();
  toFrame(L, 1, *f);

  auto f2 = (Frame**)lua_newuserdata(L, sizeof(Frame*));
  *f2 = f;

  luaL_getmetatable(L, "torchcraft.Frame");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int frameFromString(lua_State* L) {
  auto str = luaL_checkstring(L, 1);

  Frame** f = (Frame**)lua_newuserdata(L, sizeof(Frame*));
  *f = new Frame();
  std::istringstream is(str);
  is >> (**f);

  luaL_getmetatable(L, "torchcraft.Frame");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C" int frameClone(lua_State* L) {
  Frame* f = checkFrame(L);
  pushFrame(L, *f);

  Frame** f2 = (Frame**)lua_newuserdata(L, sizeof(Frame*));
  *f2 = new Frame(*f);

  luaL_getmetatable(L, "torchcraft.Frame");
  lua_setmetatable(L, -2);

  return 1;
}

// Copying from/to Lua tables

void setInt(lua_State* L, const char* key, int v) {
  lua_pushstring(L, key);
  lua_pushnumber(L, (lua_Number)v);
  lua_settable(L, -3);
}

void setBool(lua_State* L, const char* key, bool v) {
  lua_pushstring(L, key);
  lua_pushboolean(L, v);
  lua_settable(L, -3);
}

void setFlags(lua_State* L, const char* key, uint64_t flags) {
  lua_pushstring(L, key);
  lua_newtable(L);
  for (auto& it : flagNames) {
    lua_pushstring(L, it.second);
    lua_pushboolean(L, flags & it.first);
    lua_settable(L, -3);
  }
  lua_settable(L, -3);
}

// put's table[key] on top of the stack. don't forget to pop !
bool getField(lua_State* L, const char* key) {
  lua_pushstring(L, key);
  lua_gettable(L, -2);
  return (!lua_isnil(L, -1));
};

int getInt(lua_State* L, const char* key) {
  getField(L, key);
  auto res = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return res;
};

bool getBool(lua_State* L, const char* key) {
  getField(L, key);
  auto res = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return res;
};

uint64_t getFlags(lua_State* L, const char* key) {
  getField(L, key);
  uint64_t flags = 0;
  for (auto& it : flagNames) {
    flags |= getBool(L, it.second) ? it.first : 0;
  }
  lua_pop(L, 1);
  return flags;
}

void toFrame(lua_State* L, int id, Frame& res) {
  luaL_argcheck(L, lua_istable(L, id), 1, "'table' expected");
  // put's table[key] on top of the stack. don't forget to pop

  res.reward = getInt(L, "reward");
  res.is_terminal = getBool(L, "is_terminal");

  // {[playerid] = {[uid]={aid=aid, action={cmd, arg1, ...}, ...}, ...}
  getField(L, "actions");
  // iterate through player ids
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) { // -1 is the key, -2 is the table
    luaL_checktype(L, -2, LUA_TNUMBER);
    int playerId = lua_tointeger(L, -2);
    res.actions[playerId] = vector<Action>();
    auto& actions = res.actions[playerId];
    // on top of the stack is {[uid]={aid=aid, action={cmd, arg1, ...}, ...}
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      actions.push_back(Action());
      auto& action = actions.back();
      action.uid = lua_tointeger(L, -2);
      // top of the stack {aid=aid, action={}}
      action.aid = getInt(L, "aid");
      getField(L, "action");
      size_t sizeAction = lua_objlen(L, -1);
      action.action.resize(sizeAction);
      for (size_t j = 0; j < sizeAction; j++) {
        lua_rawgeti(L, -1, j + 1);
        action.action[j] = lua_tointeger(L, -1);
        lua_pop(L, 1);
      }
      lua_pop(L, 1);
      lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop the whole thing
  }
  lua_pop(L, 1); // pop the table

  // state is a table {[playerid] = {[uid]={...}, ...}, ...}
  getField(L, "state");
  // iterate through player ids
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) { // -1 is the key, -2 is the table
    luaL_checktype(L, -2, LUA_TNUMBER);
    int playerId = lua_tointeger(L, -2);
    res.units[playerId] = vector<Unit>();
    auto& units = res.units[playerId];
    // iterate through unit ids
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) { // -1 is the key, -2 is the table
      luaL_checktype(L, -2, LUA_TNUMBER);
      int unitId = lua_tointeger(L, -2);
      units.push_back(Unit());
      auto& unit = units.back();
      unit.id = unitId;

      getField(L, "position");
      lua_rawgeti(L, -1, 1);
      unit.x = lua_tointeger(L, -1);
      lua_pop(L, 1);
      lua_rawgeti(L, -1, 2);
      unit.y = lua_tointeger(L, -1);
      lua_pop(L, 1);
      lua_pop(L, 1);

      unit.health = getInt(L, "hp");
      unit.shield = getInt(L, "shield");
      unit.energy = getInt(L, "energy");
      unit.maxCD = getInt(L, "maxcd");
      unit.groundCD = getInt(L, "gwcd");
      unit.airCD = getInt(L, "awcd");
      unit.flags = getFlags(L, "flags");
      if (getBool(L, "idle") != bool(unit.flags & Unit::Flags::Idle)) {
        luaL_error(
            L,
            "inconsistent values for 'idle' in unit.flags for %d",
            unit.id);
      }
      if (getBool(L, "detected") != bool(unit.flags & Unit::Flags::Detected)) {
        luaL_error(
            L,
            "inconsistent values for 'detected' unit.flags for %d",
            unit.id);
      }
      unit.visible = getInt(L, "visible");
      unit.type = getInt(L, "type");
      unit.armor = getInt(L, "armor");
      unit.shieldArmor = getInt(L, "shieldArmor");
      unit.size = getInt(L, "size");
      // TODO pack into position_pixel
      unit.pixel_x = getInt(L, "pixel_x");
      unit.pixel_y = getInt(L, "pixel_y");
      // TODO pack into size_pixel
      unit.pixel_size_x = getInt(L, "pixel_size_x");
      unit.pixel_size_y = getInt(L, "pixel_size_y");
      unit.groundATK = getInt(L, "gwattack");
      unit.groundDmgType = getInt(L, "gwdmgtype");
      unit.groundRange = getInt(L, "gwrange");
      unit.airATK = getInt(L, "awattack");
      unit.airDmgType = getInt(L, "awdmgtype");
      unit.airRange = getInt(L, "awrange");
      unit.resources = getInt(L, "resource");

      // orders
      getField(L, "orders");
      lua_pushnil(L);
      while (lua_next(L, -2) != 0) {
        luaL_checktype(L, -2, LUA_TNUMBER);
        unit.orders.push_back(Order());

        unit.orders.back().first_frame = getInt(L, "first_frame");
        unit.orders.back().type = getInt(L, "type");
        unit.orders.back().targetId = getInt(L, "target");

        getField(L, "targetpos");
        lua_rawgeti(L, -1, 1);
        unit.orders.back().targetX = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        unit.orders.back().targetY = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_pop(L, 1);

        lua_pop(L, 1);
      }
      lua_pop(L, 1); // pop orders

      // command
      getField(L, "command");
      unit.command.frame = getInt(L, "frame");
      unit.command.type = getInt(L, "type");
      unit.command.targetId = getInt(L, "target");
      unit.command.extra = getInt(L, "extra");
      getField(L, "targetpos");
      lua_rawgeti(L, -1, 1);
      unit.command.targetX = lua_tonumber(L, -1);
      lua_pop(L, 1);
      lua_rawgeti(L, -1, 2);
      unit.command.targetY = lua_tonumber(L, -1);
      lua_pop(L, 1); // pop targetY
      lua_pop(L, 1); // pop targetpos
      lua_pop(L, 1); // pop command

      getField(L, "velocity");
      lua_rawgeti(L, -1, 1);
      unit.velocityX = lua_tonumber(L, -1);
      lua_pop(L, 1);
      lua_rawgeti(L, -1, 2);
      unit.velocityY = lua_tonumber(L, -1);
      lua_pop(L, 1);
      lua_pop(L, 1);

      unit.playerId = getInt(L, "playerId");

      lua_pop(L, 1); // pop the table
    }
    lua_pop(L, 1); // pop the whole thing
  }
  lua_pop(L, 1); // pop the state table

  // resources is a table {[playerid] = {ore=O, gas=G,
  //                                     used_psi=U, total_psi=T}, ...}
  bool success = getField(L, "resources");
  if (success) {
    // iterate through player ids
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) { // -1 is the key, -2 is the table
      luaL_checktype(L, -2, LUA_TNUMBER);
      int playerId = lua_tointeger(L, -2);
      Resources r = {getInt(L, "ore"),
                     getInt(L, "gas"),
                     getInt(L, "used_psi"),
                     getInt(L, "total_psi")};
      res.resources[playerId] = r;
      lua_pop(L, 1);
    }
  } else { // Empty resources if not provided
    for (const auto& idandlst : res.units) {
      Resources r = {0, 0, 0, 0};
      res.resources[idandlst.first] = r;
    }
  }
  lua_pop(L, 1); // pop the resources table
}

void pushResources(lua_State* L, const Resources& resources) {
  lua_newtable(L);
  setInt(L, "ore", resources.ore);
  setInt(L, "gas", resources.gas);
  setInt(L, "used_psi", resources.used_psi);
  setInt(L, "total_psi", resources.total_psi);
}

void pushUnit(lua_State* L, const Unit& unit) {
  lua_newtable(L);
  // position
  lua_pushstring(L, "position");
  lua_newtable(L);
  lua_pushnumber(L, (lua_Number)unit.x);
  lua_rawseti(L, -2, 1);
  lua_pushnumber(L, (lua_Number)unit.y);
  lua_rawseti(L, -2, 2);
  lua_settable(L, -3);

  setInt(L, "type", unit.type);
  setInt(L, "hp", unit.health);
  setInt(L, "max_hp", unit.max_health);
  setInt(L, "shield", unit.shield);
  setInt(L, "max_shield", unit.max_shield);
  setInt(L, "energy", unit.energy);
  setInt(L, "maxcd", unit.maxCD);
  setInt(L, "gwcd", unit.groundCD);
  setInt(L, "awcd", unit.airCD);
  setFlags(L, "flags", unit.flags);
  // Backwards compatibility
  setBool(L, "idle", unit.flags & Unit::Flags::Idle);
  setBool(L, "detected", unit.flags & Unit::Flags::Detected);
  setInt(L, "visible", unit.visible);
  setInt(L, "armor", unit.armor);
  setInt(L, "shieldArmor", unit.shieldArmor);
  setInt(L, "gwattack", unit.groundATK);
  setInt(L, "awattack", unit.airATK);
  setInt(L, "size", unit.size);
  setInt(L, "pixel_x", unit.pixel_x);
  setInt(L, "pixel_y", unit.pixel_y);
  setInt(L, "pixel_size_x", unit.pixel_size_x);
  setInt(L, "pixel_size_y", unit.pixel_size_y);
  setInt(L, "gwdmgtype", unit.groundDmgType);
  setInt(L, "awdmgtype", unit.airDmgType);
  setInt(L, "gwrange", unit.groundRange);
  setInt(L, "awrange", unit.airRange);
  setInt(L, "resource", unit.resources);

  // orders
  lua_pushstring(L, "orders");
  lua_newtable(L);
  for (unsigned i = 0; i < unit.orders.size(); i++) {
    lua_newtable(L);
    setInt(L, "first_frame", unit.orders[i].first_frame);
    setInt(L, "type", unit.orders[i].type);
    setInt(L, "target", unit.orders[i].targetId);

    lua_pushstring(L, "targetpos");
    lua_newtable(L);
    lua_pushnumber(L, (lua_Number)unit.orders[i].targetX);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, (lua_Number)unit.orders[i].targetY);
    lua_rawseti(L, -2, 2);
    lua_settable(L, -3);

    lua_rawseti(L, -2, i + 1);
  }
  lua_settable(L, -3);

  // command
  lua_pushstring(L, "command");
  lua_newtable(L);
  setInt(L, "frame", unit.command.frame);
  setInt(L, "type", unit.command.type);
  setInt(L, "target", unit.command.targetId);
  setInt(L, "extra", unit.command.extra);
  lua_pushstring(L, "targetpos");
  lua_newtable(L);
  lua_pushnumber(L, (lua_Number)unit.command.targetX);
  lua_rawseti(L, -2, 1);
  lua_pushnumber(L, (lua_Number)unit.command.targetY);
  lua_rawseti(L, -2, 2);
  lua_settable(L, -3);
  lua_settable(L, -3);

  // For compatibility {
  // This block can be removed safely once Lua code no longer depends
  // on the order, target and targetpos fields of units.
  Order o = {0 /* frame number is not important */,
             189 /* BWAPI::Orders::Enum::None */,
             -1,
             -1,
             -1};
  if (!unit.orders.empty())
    o = unit.orders.back();

  setInt(L, "order", o.type);
  setInt(L, "target", o.targetId);

  lua_pushstring(L, "targetpos");
  lua_newtable(L);
  lua_pushnumber(L, (lua_Number)o.targetX);
  lua_rawseti(L, -2, 1);
  lua_pushnumber(L, (lua_Number)o.targetY);
  lua_rawseti(L, -2, 2);
  lua_settable(L, -3);
  // }

  lua_pushstring(L, "velocity");
  lua_newtable(L);
  lua_pushnumber(L, (lua_Number)unit.velocityX);
  lua_rawseti(L, -2, 1);
  lua_pushnumber(L, (lua_Number)unit.velocityY);
  lua_rawseti(L, -2, 2);
  lua_settable(L, -3);

  setInt(L, "playerId", unit.playerId);
}

void pushFrame(lua_State* L, const Frame& res) {
  lua_newtable(L);
  setInt(L, "reward", res.reward);
  setBool(L, "is_terminal", res.is_terminal);

  // actions is a table {[playerid] = {{cmd, arg1, ...}, ...}, ...}
  lua_pushstring(L, "actions");
  lua_newtable(L);
  for (const auto& player : res.actions) {
    lua_newtable(L);
    for (const auto& action : player.second) {
      lua_newtable(L);

      lua_pushstring(L, "aid");
      lua_pushnumber(L, (lua_Number)action.aid);
      lua_settable(L, -3);

      lua_pushstring(L, "action");
      lua_newtable(L);
      int idCommand = 1;
      for (auto commandArg : action.action) {
        lua_pushnumber(L, (lua_Number)commandArg);
        lua_rawseti(L, -2, idCommand++);
      }
      lua_settable(L, -3);

      lua_rawseti(L, -2, action.uid);
    }
    lua_rawseti(L, -2, player.first);
  }
  lua_settable(L, -3);

  // state is a table {[playerid] = {[uid]={...}, ...}, ...}
  lua_pushstring(L, "state");
  lua_newtable(L);
  for (const auto& player : res.units) {
    lua_newtable(L);
    for (const auto& unit : player.second) {
      pushUnit(L, unit);
      lua_rawseti(L, -2, unit.id);
    }
    lua_rawseti(L, -2, player.first);
  }
  lua_settable(L, -3);
  // resources is a table {[playerid] = {ore=O, gas=G,
  //                                     used_psi=U, total_psi=T}, ...}
  lua_pushstring(L, "resources");
  lua_newtable(L);
  for (const auto& player : res.resources) {
    pushResources(L, player.second);
    lua_rawseti(L, -2, player.first);
  }
  lua_settable(L, -3);
}

extern "C" int frameGetUnits(lua_State* L) {
  Frame* f = checkFrame(L);
  size_t playerId = luaL_checkint(L, 2);

  // create an array of tables (units)
  lua_newtable(L);

  // then for each unit, add it to the array
  if (f->units.count(playerId) == 0)
    return 1;

  for (const auto& unit : f->units.at(playerId)) {
    lua_pushnumber(L, (lua_Number)unit.id);
    pushUnit(L, unit);
    lua_settable(L, -3);
  }

  return 1;
}

extern "C" int frameGetResources(lua_State* L) {
  Frame* f = checkFrame(L);
  size_t playerId = luaL_checkint(L, 2);
  if (f->resources.find(playerId) == f->resources.end())
    return 1;
  const auto& r = f->resources.at(playerId);
  pushResources(L, r);
  return 1;
}

extern "C" int frameGetNumUnits(lua_State* L) {
  Frame* f = checkFrame(L);
  size_t n = 0;
  if (lua_isnoneornil(L, 2)) {
    for (const auto& units : f->units) {
      n += units.second.size();
    }
  } else {
    n = f->units[luaL_checkint(L, 2)].size();
  }
  lua_pushnumber(L, (lua_Number)n);
  return 1;
}

extern "C" int frameGetNumPlayers(lua_State* L) {
  Frame* f = checkFrame(L);
  lua_pushnumber(L, (lua_Number)f->units.size());
  return 1;
}

extern "C" int frameToTable(lua_State* L) {
  Frame* f = checkFrame(L);
  pushFrame(L, *f);

  return 1;
}

extern "C" int frameToString(lua_State* L) {
  Frame* f = checkFrame(L);
  std::ostringstream out;
  out << (*f);

  lua_pushstring(L, out.str().c_str());

  return 1;
}

extern "C" int frameCombine(lua_State* L) {
  Frame* f = checkFrame(L);
  Frame* f2 = checkFrame(L, 2);

  f->combine(*f2);

  // return f
  lua_pushvalue(L, 1);
  return 1;
}

extern "C" int frameDeepEq(lua_State* L) {
  // Optional third argument supresses debug info if it's false
  Frame* f = checkFrame(L);
  Frame* f2 = checkFrame(L, 2);
  bool good;
  if (lua_isnil(L, 3)) {
    good = detail::frameEq(f, f2);
  } else {
    good = detail::frameEq(f, f2, lua_toboolean(L, 3));
  }
  lua_pushboolean(L, good);
  return 1;
}
