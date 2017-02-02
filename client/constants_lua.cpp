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

#include "constants.h"
#include "constants_lua.h"
#include "lua_utils.h"

using client::lua::pushValue;
using client::lua::sealTable;

namespace {

std::string fromCamelCaseToLower(const std::string& s) {
  if (s == "MAX") {
    return s;
  }

  std::ostringstream ss;
  auto it = s.begin();
  ss << char(tolower(*it++));
  while (it != s.end()) {
    if (isupper(*it)) {
      ss << '_' << char(tolower(*it++));
    } else {
      ss << *it++;
    }
  }
  return ss.str();
}

int wisBuilding(lua_State* L) {
  int n = luaL_checkint(L, lua_gettop(L) > 1 ? 2 : 1);
  auto id = client::BW::UnitType::_from_integral_nothrow(n);
  if (!id) {
    return luaL_error(L, "Invalid unit ID: %d", n);
  }
  lua_pushboolean(L, client::BW::isBuilding(*id));
  return 1;
}

int wisWorker(lua_State* L) {
  int n = luaL_checkint(L, lua_gettop(L) > 1 ? 2 : 1);
  auto id = client::BW::UnitType::_from_integral_nothrow(n);
  if (!id) {
    return luaL_error(L, "Invalid unit ID: %d", n);
  }
  lua_pushboolean(L, client::BW::isWorker(*id));
  return 1;
}

int wisMineralField(lua_State* L) {
  int n = luaL_checkint(L, lua_gettop(L) > 1 ? 2 : 1);
  auto id = client::BW::UnitType::_from_integral_nothrow(n);
  if (!id) {
    return luaL_error(L, "Invalid unit ID: %d", n);
  }
  lua_pushboolean(L, client::BW::isMineralField(*id));
  return 1;
}

int wisGasGeyser(lua_State* L) {
  int n = luaL_checkint(L, lua_gettop(L) > 1 ? 2 : 1);
  auto id = client::BW::UnitType::_from_integral_nothrow(n);
  if (!id) {
    return luaL_error(L, "Invalid unit ID: %d", n);
  }
  lua_pushboolean(L, client::BW::isGasGeyser(*id));
  return 1;
}

template <typename Enum>
void pushTable(lua_State* L) {
  lua_newtable(L);
  for (auto elem : Enum::_values()) {
    lua_pushinteger(L, elem._to_integral());
    lua_setfield(L, -2, elem._to_string());
    lua_pushstring(L, elem._to_string());
    lua_rawseti(L, -2, elem._to_integral());
  }
  sealTable(L);
}

template <typename Enum, typename F>
void pushTable(lua_State* L, F map) {
  lua_newtable(L);
  for (auto elem : Enum::_values()) {
    lua_pushinteger(L, elem._to_integral());
    auto mapped = map(elem._to_string());
    lua_setfield(L, -2, mapped.c_str());
    lua_pushstring(L, mapped.c_str());
    lua_rawseti(L, -2, elem._to_integral());
  }
  sealTable(L);
}

template <typename Enum>
void pushVector(lua_State* L, std::vector<Enum> v) {
  lua_newtable(L);
  for (size_t i = 0; i < v.size(); i++) {
    lua_pushinteger(L, v[i]._to_integral());
    lua_rawseti(L, -2, i + 1);
  }
  sealTable(L);
}

template <typename T>
void pushStaticValues(lua_State* L, const T m[]) {
  lua_newtable(L);
  for (size_t i = 0; i < client::BW::data::NumKeys; i++) {
    const auto& key = client::BW::data::Keys[i];
    pushValue(L, m[i]);
    lua_setfield(L, -2, key.c_str());

    auto t = client::BW::UnitType::_from_string_nothrow(key.c_str());
    if (t) {
      pushValue(L, m[i]);
      lua_rawseti(L, -2, t->_to_integral());
    }
  }
  sealTable(L);
}

void pushMap(lua_State* L, std::unordered_map<client::BW::UnitType, int>& m) {
  lua_newtable(L);
  for (const auto& kv : m) {
    pushValue(L, kv.second);
    lua_rawseti(L, -2, kv.first._to_integral());
  }
  sealTable(L);
}

void pushStaticData(lua_State* L) {
  lua_newtable(L);
  pushStaticValues(L, client::BW::data::CanAttack);
  lua_setfield(L, -2, "canAttack");
  pushStaticValues(L, client::BW::data::DimensionRight);
  lua_setfield(L, -2, "dimensionRight");
  pushStaticValues(L, client::BW::data::Height);
  lua_setfield(L, -2, "height");
  pushStaticValues(L, client::BW::data::IsMineralField);
  lua_setfield(L, -2, "isMineralField");
  pushStaticValues(L, client::BW::data::CanProduce);
  lua_setfield(L, -2, "canProduce");
  pushStaticValues(L, client::BW::data::IsRefinery);
  lua_setfield(L, -2, "isRefinery");
  pushStaticValues(L, client::BW::data::IsResourceDepot);
  lua_setfield(L, -2, "isResourceDepot");
  pushStaticValues(L, client::BW::data::RegeneratesHP);
  lua_setfield(L, -2, "regeneratesHP");
  pushStaticValues(L, client::BW::data::IsCloakable);
  lua_setfield(L, -2, "isCloakable");
  pushStaticValues(L, client::BW::data::IsTwoUnitsInOneEgg);
  lua_setfield(L, -2, "isTwoUnitsInOneEgg");
  pushStaticValues(L, client::BW::data::IsSpellcaster);
  lua_setfield(L, -2, "isSpellcaster");
  pushStaticValues(L, client::BW::data::SupplyRequired);
  lua_setfield(L, -2, "supplyRequired");
  pushStaticValues(L, client::BW::data::AirWeapon);
  lua_setfield(L, -2, "airWeapon");
  pushStaticValues(L, client::BW::data::BuildScore);
  lua_setfield(L, -2, "buildScore");
  pushStaticValues(L, client::BW::data::MaxAirHits);
  lua_setfield(L, -2, "maxAirHits");
  pushStaticValues(L, client::BW::data::IsPowerup);
  lua_setfield(L, -2, "isPowerup");
  pushStaticValues(L, client::BW::data::IsBeacon);
  lua_setfield(L, -2, "isBeacon");
  pushStaticValues(L, client::BW::data::MineralPrice);
  lua_setfield(L, -2, "mineralPrice");
  pushStaticValues(L, client::BW::data::IsInvincible);
  lua_setfield(L, -2, "isInvincible");
  pushStaticValues(L, client::BW::data::RequiredTech);
  lua_setfield(L, -2, "requiredTech");
  pushStaticValues(L, client::BW::data::DimensionDown);
  lua_setfield(L, -2, "dimensionDown");
  pushStaticValues(L, client::BW::data::CanBuildAddon);
  lua_setfield(L, -2, "canBuildAddon");
  pushStaticValues(L, client::BW::data::DimensionLeft);
  lua_setfield(L, -2, "dimensionLeft");
  pushStaticValues(L, client::BW::data::ProducesLarva);
  lua_setfield(L, -2, "producesLarva");
  pushStaticValues(L, client::BW::data::Armor);
  lua_setfield(L, -2, "armor");
  pushStaticValues(L, client::BW::data::IsMechanical);
  lua_setfield(L, -2, "isMechanical");
  pushStaticValues(L, client::BW::data::IsBuilding);
  lua_setfield(L, -2, "isBuilding");
  pushStaticValues(L, client::BW::data::SupplyProvided);
  lua_setfield(L, -2, "supplyProvided");
  pushStaticValues(L, client::BW::data::SightRange);
  lua_setfield(L, -2, "sightRange");
  pushStaticValues(L, client::BW::data::GasPrice);
  lua_setfield(L, -2, "gasPrice");
  pushStaticValues(L, client::BW::data::MaxHitPoints);
  lua_setfield(L, -2, "maxHitPoints");
  pushStaticValues(L, client::BW::data::Width);
  lua_setfield(L, -2, "width");
  pushStaticValues(L, client::BW::data::TileWidth);
  lua_setfield(L, -2, "tileWidth");
  pushStaticValues(L, client::BW::data::IsHero);
  lua_setfield(L, -2, "isHero");
  pushStaticValues(L, client::BW::data::SeekRange);
  lua_setfield(L, -2, "seekRange");
  pushStaticValues(L, client::BW::data::BuildTime);
  lua_setfield(L, -2, "buildTime");
  pushStaticValues(L, client::BW::data::IsCritter);
  lua_setfield(L, -2, "isCritter");
  pushStaticValues(L, client::BW::data::RequiresPsi);
  lua_setfield(L, -2, "requiresPsi");
  pushStaticValues(L, client::BW::data::IsSpecialBuilding);
  lua_setfield(L, -2, "isSpecialBuilding");
  pushStaticValues(L, client::BW::data::GroundWeapon);
  lua_setfield(L, -2, "groundWeapon");
  pushStaticValues(L, client::BW::data::IsFlyer);
  lua_setfield(L, -2, "isFlyer");
  pushStaticValues(L, client::BW::data::Size);
  lua_setfield(L, -2, "size");
  pushStaticValues(L, client::BW::data::IsNeutral);
  lua_setfield(L, -2, "isNeutral");
  pushStaticValues(L, client::BW::data::MaxShields);
  lua_setfield(L, -2, "maxShields");
  pushStaticValues(L, client::BW::data::HasPermanentCloak);
  lua_setfield(L, -2, "hasPermanentCloak");
  pushStaticValues(L, client::BW::data::TopSpeed);
  lua_setfield(L, -2, "topSpeed");
  pushStaticValues(L, client::BW::data::TileHeight);
  lua_setfield(L, -2, "tileHeight");
  pushStaticValues(L, client::BW::data::IsRobotic);
  lua_setfield(L, -2, "isRobotic");
  pushStaticValues(L, client::BW::data::DimensionUp);
  lua_setfield(L, -2, "dimensionUp");
  pushStaticValues(L, client::BW::data::DestroyScore);
  lua_setfield(L, -2, "destroyScore");
  pushStaticValues(L, client::BW::data::SpaceProvided);
  lua_setfield(L, -2, "spaceProvided");
  pushStaticValues(L, client::BW::data::TileSize);
  lua_setfield(L, -2, "tileSize");
  pushStaticValues(L, client::BW::data::HaltDistance);
  lua_setfield(L, -2, "haltDistance");
  pushStaticValues(L, client::BW::data::IsAddon);
  lua_setfield(L, -2, "isAddon");
  pushStaticValues(L, client::BW::data::CanMove);
  lua_setfield(L, -2, "canMove");
  pushStaticValues(L, client::BW::data::IsFlyingBuilding);
  lua_setfield(L, -2, "isFlyingBuilding");
  pushStaticValues(L, client::BW::data::MaxEnergy);
  lua_setfield(L, -2, "maxEnergy");
  pushStaticValues(L, client::BW::data::IsDetector);
  lua_setfield(L, -2, "isDetector");
  pushStaticValues(L, client::BW::data::IsOrganic);
  lua_setfield(L, -2, "isOrganic");
  pushStaticValues(L, client::BW::data::SpaceRequired);
  lua_setfield(L, -2, "spaceRequired");
  pushStaticValues(L, client::BW::data::IsFlagBeacon);
  lua_setfield(L, -2, "isFlagBeacon");
  pushStaticValues(L, client::BW::data::IsWorker);
  lua_setfield(L, -2, "isWorker");
  pushStaticValues(L, client::BW::data::IsBurrowable);
  lua_setfield(L, -2, "isBurrowable");
  pushStaticValues(L, client::BW::data::CloakingTech);
  lua_setfield(L, -2, "cloakingTech");
  pushStaticValues(L, client::BW::data::IsResourceContainer);
  lua_setfield(L, -2, "isResourceContainer");
  pushStaticValues(L, client::BW::data::Acceleration);
  lua_setfield(L, -2, "acceleration");
  pushStaticValues(L, client::BW::data::IsSpell);
  lua_setfield(L, -2, "isSpell");
  pushStaticValues(L, client::BW::data::RequiresCreep);
  lua_setfield(L, -2, "requiresCreep");
  pushStaticValues(L, client::BW::data::ArmorUpgrade);
  lua_setfield(L, -2, "armorUpgrade");
  pushStaticValues(L, client::BW::data::MaxGroundHits);
  lua_setfield(L, -2, "maxGroundHits");
  pushStaticValues(L, client::BW::data::TurnRadius);
  lua_setfield(L, -2, "turnRadius");
  pushStaticValues(L, client::BW::data::GetRace);
  lua_setfield(L, -2, "getRace");
}

} // namespace

namespace client {
void registerConstants(lua_State* L, int index) {
  lua_pushvalue(L, index);
  lua_newtable(L);

  pushTable<BW::Command>(L, fromCamelCaseToLower);
  lua_setfield(L, -2, "commands");
  pushTable<BW::UserCommandType>(L);
  lua_setfield(L, -2, "usercommandtypes");
  pushTable<BW::UnitCommandType>(L);
  lua_setfield(L, -2, "unitcommandtypes");
  pushTable<BW::Order>(L);
  lua_setfield(L, -2, "orders");
  pushTable<BW::TechType>(L);
  lua_setfield(L, -2, "techtypes");
  pushTable<BW::UnitType>(L);
  lua_setfield(L, -2, "unittypes");
  pushTable<BW::BulletType>(L);
  lua_setfield(L, -2, "bullettypes");
  pushTable<BW::WeaponType>(L);
  lua_setfield(L, -2, "weapontypes");
  pushTable<BW::UnitSize>(L);
  lua_setfield(L, -2, "unitsizes");
  pushTable<BW::DamageType>(L);
  lua_setfield(L, -2, "dmgtypes");

  lua_newtable(L);
  for (auto t : BW::UnitType::_values()) {
    auto v = BW::unitProductions(t);
    if (!v.empty()) {
      pushVector(L, std::move(v));
      lua_rawseti(L, -2, t._to_integral());
    }
  }
  sealTable(L);
  lua_setfield(L, -2, "produces");

  lua_newtable(L);
  for (auto t : BW::UnitType::_values()) {
    for (auto prod : BW::unitProductions(t)) {
      lua_pushinteger(L, t._to_integral());
      lua_rawseti(L, -2, prod._to_integral());
    }
  }
  sealTable(L);
  lua_setfield(L, -2, "isproducedby");

  lua_newtable(L);
  for (auto t : BW::UnitCommandType::_values()) {
    auto v = BW::commandToOrders(t);
    if (!v.empty()) {
      pushVector(L, std::move(v));
      lua_rawseti(L, -2, t._to_integral());
    }
  }
  sealTable(L);
  lua_setfield(L, -2, "command2order");

  lua_newtable(L);
  for (auto t : BW::UnitCommandType::_values()) {
    for (auto o : BW::commandToOrders(t)) {
      lua_rawgeti(L, -1, o._to_integral());
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_rawseti(L, -2, o._to_integral());
        lua_rawgeti(L, -1, o._to_integral());
      }
      auto size = lua_objlen(L, -1);
      lua_pushinteger(L, t._to_integral());
      lua_rawseti(L, -2, size + 1);
      lua_pop(L, 1);
    }
  }
  sealTable(L);
  lua_setfield(L, -2, "order2command");

  lua_pushcfunction(L, wisBuilding);
  lua_setfield(L, -2, "isbuilding");
  lua_pushcfunction(L, wisWorker);
  lua_setfield(L, -2, "isworker");
  lua_pushcfunction(L, wisMineralField);
  lua_setfield(L, -2, "is_mineral_field");
  lua_pushcfunction(L, wisGasGeyser);
  lua_setfield(L, -2, "is_gas_geyser");

  pushStaticData(L);
  sealTable(L);
  lua_setfield(L, -2, "staticdata");

  // total_price
  lua_newtable(L);
  pushMap(L, client::BW::data::TotalMineralPrice);
  lua_setfield(L, -2, "mineral");
  pushMap(L, client::BW::data::TotalGasPrice);
  lua_setfield(L, -2, "gas");
  lua_setfield(L, -2, "total_price");

  lua_setfield(L, -2, "const");
  lua_pop(L, 1);
}
} // namespace client
