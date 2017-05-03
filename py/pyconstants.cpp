#include "pytorchcraft.h"

#include "constants.h"

#include <sstream>

using namespace torchcraft;

// We have to use a class for BetterEnums
template <typename Enum, typename F>
void setEnumDict(py::module& module, const std::string& name, F map) {
  py::module mod = module.def_submodule(name.c_str());
  py::dict dict;
  for (auto v : Enum::_values()) {
    auto str = map(v._to_string());
    dict[py::str(str)] = py::int_(v._to_integral());
    dict[py::int_(v._to_integral())] = py::str(str);
    mod.attr(py::str(str)) = py::int_(v._to_integral());
  }
  mod.attr("_dict") = dict;
}

template <typename Enum>
void setEnumDict(py::module& module, const std::string& name) {
  setEnumDict<Enum>(module, name, [](const std::string& s) { return s; });
}

template <typename Enum>
std::vector<int32_t> getEnumVector(std::vector<Enum> v) {
  std::vector<int32_t> lst;
  for (size_t i = 0; i < v.size(); i++)
    lst.push_back(v[i]._to_integral());
  return lst;
}

template <typename T>
py::dict getStaticValues(const T m[]) {
  py::dict dict;
  for (auto ut : BW::UnitType::_values()) {
    if (ut._to_integral() == BW::UnitType::MAX)
      continue;
    dict[py::str(ut._to_string())] = m[ut];
    dict[py::int_(ut._to_integral())] = m[ut];
  }
  return dict;
}

void init_constants(py::module& torchcraft) {
  py::module constants = torchcraft.def_submodule("Constants");
  setEnumDict<BW::Command>(constants, "commands", fromCamelCaseToLower);
  for (auto v : BW::Command::_values())
    constants.attr(fromCamelCaseToLower(v._to_string()).c_str()) =
        v._to_integral();
  setEnumDict<BW::UnitCommandType>(constants, "unitcommandtypes");
  setEnumDict<BW::Order>(constants, "orders");
  setEnumDict<BW::TechType>(constants, "techtypes");
  setEnumDict<BW::UnitType>(constants, "unittypes");
  setEnumDict<BW::BulletType>(constants, "bullettypes");
  setEnumDict<BW::WeaponType>(constants, "weapontypes");
  setEnumDict<BW::UnitSize>(constants, "unitsizes");
  setEnumDict<BW::DamageType>(constants, "dmgtypes");
  setEnumDict<BW::Color>(constants, "colors");

  py::dict produces, producedby;
  for (auto t : BW::UnitType::_values()) {
    if (t._to_integral() == BW::UnitType::MAX)
      continue;
    auto v = BW::unitProductions(t);
    if (!v.empty())
      produces[py::int_(t._to_integral())] = getEnumVector(std::move(v));
    for (auto prod : v)
      producedby[py::int_(t._to_integral())] = py::int_(prod._to_integral());
  }
  constants.attr("produces") = produces; // Dict of UnitType: [productions]
  constants.attr("isproducedby") = producedby; // Dict of UnitType: producedBy

  py::dict c2o;
  for (auto t : BW::UnitCommandType::_values()) {
    auto v = BW::commandToOrders(t);
    if (!v.empty())
      c2o[py::int_(t._to_integral())] = getEnumVector(std::move(v));
  }
  constants.attr("command2order") = c2o;

  std::unordered_map<int32_t, std::vector<int32_t>> o2c;
  for (auto t : BW::UnitCommandType::_values()) {
    for (auto o : BW::commandToOrders(t)) {
      auto i = o._to_integral();
      if (o2c.find(i) == o2c.end())
        o2c[i] = std::vector<int32_t>();
      o2c[i].push_back(t._to_integral());
    }
  }
  constants.attr("order2command") = o2c;

  constants.def("isbuilding", [](int32_t id) {
    return BW::isBuilding(BW::UnitType::_from_integral(id));
  });
  constants.def("isworker", [](int32_t id) {
    return BW::isWorker(BW::UnitType::_from_integral(id));
  });
  constants.def("is_mineral_field", [](int32_t id) {
    return BW::isMineralField(BW::UnitType::_from_integral(id));
  });
  constants.def("is_gas_geyser", [](int32_t id) {
    return BW::isGasGeyser(BW::UnitType::_from_integral(id));
  });

  py::dict sv;
  sv["canAttack"] = getStaticValues(BW::data::CanAttack);
  sv["dimensionRight"] = getStaticValues(BW::data::DimensionRight);
  sv["height"] = getStaticValues(BW::data::Height);
  sv["isMineralField"] = getStaticValues(BW::data::IsMineralField);
  sv["canProduce"] = getStaticValues(BW::data::CanProduce);
  sv["isRefinery"] = getStaticValues(BW::data::IsRefinery);
  sv["isResourceDepot"] = getStaticValues(BW::data::IsResourceDepot);
  sv["regeneratesHP"] = getStaticValues(BW::data::RegeneratesHP);
  sv["isCloakable"] = getStaticValues(BW::data::IsCloakable);
  sv["isTwoUnitsInOneEgg"] = getStaticValues(BW::data::IsTwoUnitsInOneEgg);
  sv["isSpellcaster"] = getStaticValues(BW::data::IsSpellcaster);
  sv["supplyRequired"] = getStaticValues(BW::data::SupplyRequired);
  sv["airWeapon"] = getStaticValues(BW::data::AirWeapon);
  sv["buildScore"] = getStaticValues(BW::data::BuildScore);
  sv["maxAirHits"] = getStaticValues(BW::data::MaxAirHits);
  sv["isPowerup"] = getStaticValues(BW::data::IsPowerup);
  sv["isBeacon"] = getStaticValues(BW::data::IsBeacon);
  sv["mineralPrice"] = getStaticValues(BW::data::MineralPrice);
  sv["isInvincible"] = getStaticValues(BW::data::IsInvincible);
  sv["requiredTech"] = getStaticValues(BW::data::RequiredTech);
  sv["dimensionDown"] = getStaticValues(BW::data::DimensionDown);
  sv["canBuildAddon"] = getStaticValues(BW::data::CanBuildAddon);
  sv["dimensionLeft"] = getStaticValues(BW::data::DimensionLeft);
  sv["producesLarva"] = getStaticValues(BW::data::ProducesLarva);
  sv["armor"] = getStaticValues(BW::data::Armor);
  sv["isMechanical"] = getStaticValues(BW::data::IsMechanical);
  sv["isBuilding"] = getStaticValues(BW::data::IsBuilding);
  sv["supplyProvided"] = getStaticValues(BW::data::SupplyProvided);
  sv["sightRange"] = getStaticValues(BW::data::SightRange);
  sv["gasPrice"] = getStaticValues(BW::data::GasPrice);
  sv["maxHitPoints"] = getStaticValues(BW::data::MaxHitPoints);
  sv["width"] = getStaticValues(BW::data::Width);
  sv["tileWidth"] = getStaticValues(BW::data::TileWidth);
  sv["isHero"] = getStaticValues(BW::data::IsHero);
  sv["seekRange"] = getStaticValues(BW::data::SeekRange);
  sv["buildTime"] = getStaticValues(BW::data::BuildTime);
  sv["isCritter"] = getStaticValues(BW::data::IsCritter);
  sv["requiresPsi"] = getStaticValues(BW::data::RequiresPsi);
  sv["isSpecialBuilding"] = getStaticValues(BW::data::IsSpecialBuilding);
  sv["groundWeapon"] = getStaticValues(BW::data::GroundWeapon);
  sv["isFlyer"] = getStaticValues(BW::data::IsFlyer);
  sv["size"] = getStaticValues(BW::data::Size);
  sv["isNeutral"] = getStaticValues(BW::data::IsNeutral);
  sv["maxShields"] = getStaticValues(BW::data::MaxShields);
  sv["hasPermanentCloak"] = getStaticValues(BW::data::HasPermanentCloak);
  sv["topSpeed"] = getStaticValues(BW::data::TopSpeed);
  sv["tileHeight"] = getStaticValues(BW::data::TileHeight);
  sv["isRobotic"] = getStaticValues(BW::data::IsRobotic);
  sv["dimensionUp"] = getStaticValues(BW::data::DimensionUp);
  sv["destroyScore"] = getStaticValues(BW::data::DestroyScore);
  sv["spaceProvided"] = getStaticValues(BW::data::SpaceProvided);
  sv["tileSize"] = getStaticValues(BW::data::TileSize);
  sv["haltDistance"] = getStaticValues(BW::data::HaltDistance);
  sv["isAddon"] = getStaticValues(BW::data::IsAddon);
  sv["canMove"] = getStaticValues(BW::data::CanMove);
  sv["isFlyingBuilding"] = getStaticValues(BW::data::IsFlyingBuilding);
  sv["maxEnergy"] = getStaticValues(BW::data::MaxEnergy);
  sv["isDetector"] = getStaticValues(BW::data::IsDetector);
  sv["isOrganic"] = getStaticValues(BW::data::IsOrganic);
  sv["spaceRequired"] = getStaticValues(BW::data::SpaceRequired);
  sv["isFlagBeacon"] = getStaticValues(BW::data::IsFlagBeacon);
  sv["isWorker"] = getStaticValues(BW::data::IsWorker);
  sv["isBurrowable"] = getStaticValues(BW::data::IsBurrowable);
  sv["cloakingTech"] = getStaticValues(BW::data::CloakingTech);
  sv["isResourceContainer"] = getStaticValues(BW::data::IsResourceContainer);
  sv["acceleration"] = getStaticValues(BW::data::Acceleration);
  sv["isSpell"] = getStaticValues(BW::data::IsSpell);
  sv["requiresCreep"] = getStaticValues(BW::data::RequiresCreep);
  sv["armorUpgrade"] = getStaticValues(BW::data::ArmorUpgrade);
  sv["maxGroundHits"] = getStaticValues(BW::data::MaxGroundHits);
  sv["turnRadius"] = getStaticValues(BW::data::TurnRadius);
  sv["getRace"] = getStaticValues(BW::data::GetRace);

  constants.attr("staticvalues") = sv;

  py::dict tp, totmin, totgas;
  for (auto u : BW::data::TotalMineralPrice)
    totmin[py::int_(u.first._to_integral())] = u.second;
  for (auto u : BW::data::TotalGasPrice)
    totgas[py::int_(u.first._to_integral())] = u.second;
  tp["mineral"] = totmin;
  tp["gas"] = totgas;
}
