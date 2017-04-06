#include "pytorchcraft.h"

#include "constants.h"

#include <sstream>

using namespace torchcraft;

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

// We have to use a class for BetterEnums
template <typename Enum, typename F>
void setEnumDict(py::module &module, const std::string &name, F map) {
  py::dict dict;
  for (auto v : Enum::_values()) {
    auto str = map(v._to_string());
    dict[py::str(str)] = py::int_(v._to_integral());
    dict[py::int_(v._to_integral())] = py::str(str);
  }
  module.attr(name.c_str()) = dict;
}

template <typename Enum>
void setEnumDict(py::module &module, const std::string &name) {
  setEnumDict<Enum>(module, name, [](const std::string& s) { return s; });
}

template <typename Enum>
std::vector<int32_t> getEnumVector(std::vector<Enum> v) {
  std::vector<int32_t> lst;
  for (size_t i = 0; i < v.size(); i++) lst.push_back(v[i]._to_integral());
  return lst;
}

void init_constants(py::module &torchcraft) {
  setEnumDict<BW::Command>(torchcraft, "commands", fromCamelCaseToLower);
  setEnumDict<BW::UnitCommandType>(torchcraft, "unitcommandtypes");
  setEnumDict<BW::Order>(torchcraft, "orders");
  setEnumDict<BW::TechType>(torchcraft, "techtypes");
  setEnumDict<BW::UnitType>(torchcraft, "unittypes");
  setEnumDict<BW::BulletType>(torchcraft, "bullettypes");
  setEnumDict<BW::WeaponType>(torchcraft, "weapontypes");
  setEnumDict<BW::UnitSize>(torchcraft, "unitsizes");
  setEnumDict<BW::DamageType>(torchcraft, "dmgtypes");
  setEnumDict<BW::Color>(torchcraft, "colors");

  py::dict produces, producedby;
  for (auto t : BW::UnitType::_values()) {
    auto v = BW::unitProductions(t);
    if (!v.empty())
      produces[py::int_(t._to_integral())] = getEnumVector(std::move(v));
    for (auto prod : v)
      producedby[py::int_(t._to_integral())] = py::int_(prod._to_integral());
  }
  torchcraft.attr("produces") = produces; // Dict of UnitType: [productions]
  torchcraft.attr("isproducedby") = producedby; // Dict of UnitType: producedBy

  py::dict c2o;
  for (auto t : BW::UnitCommandType::_values()) {
    auto v = BW::commandToOrders(t);
    if (!v.empty())
      c2o[py::int_(t._to_integral())] = getEnumVector(std::move(v));
  }
  torchcraft.attr("command2order") = c2o;

  std::unordered_map<int32_t, std::vector<int32_t>> o2c;
  for (auto t : BW::UnitCommandType::_values()) {
    for (auto o : BW::commandToOrders(t)) {
      auto i = o._to_integral();
      if (o2c.find(i) == o2c.end()) o2c[i] = std::vector<int32_t>();
      o2c[i].push_back(t._to_integral());
    }
  }
  torchcraft.attr("order2command") = o2c;

  torchcraft.def("isbuilding", [](int32_t id) {
      return BW::isBuilding(BW::UnitType::_from_integral(id)); });
  torchcraft.def("isworker", [](int32_t id) {
      return BW::isWorker(BW::UnitType::_from_integral(id)); });
  torchcraft.def("is_mineral_field", [](int32_t id) {
      return BW::isMineralField(BW::UnitType::_from_integral(id)); });
  torchcraft.def("is_gas_geyser", [](int32_t id) {
      return BW::isGasGeyser(BW::UnitType::_from_integral(id)); });

  /* TODO
  pushStaticData(L);
  sealTable(L);
  lua_setfield(L, -2, "staticdata");

  // total_price
  lua_newtable(L);
  pushMap(L, torchcraft::BW::data::TotalMineralPrice);
  lua_setfield(L, -2, "mineral");
  pushMap(L, torchcraft::BW::data::TotalGasPrice);
  lua_setfield(L, -2, "gas");
  lua_setfield(L, -2, "total_price");

  lua_setfield(L, -2, "const");
  lua_pop(L, 1);
  */
}
