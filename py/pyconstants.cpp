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
void setEnum(py::class_<Enum> e, F map) {
  std::vector <std::string> attributes;
  for (auto v : Enum::_values()) {
    auto str = map(v._to_string()).c_str();
    attributes.push_back(str);
    e.def_property_readonly_static(str, [&v]() { return v._to_integral(); });
  }
  e.def("_to_list", [attributes]() { return attributes; });
}

template <typename Enum>
void setEnum(py::class_<Enum> e) {
  setEnum(e, [](const std::string& s) { return s; });
}

void init_constants(py::module &torchcraft) {
  setEnum(py::class_<BW::Command>(torchcraft, "commands"), fromCamelCaseToLower);
  setEnum(py::class_<BW::UnitCommandType>(torchcraft, "unitcommandtypes"));
  setEnum(py::class_<BW::Order>(torchcraft, "orders"));
  setEnum(py::class_<BW::TechType>(torchcraft, "techtypes"));
  setEnum(py::class_<BW::UnitType>(torchcraft, "unittypes"));
  setEnum(py::class_<BW::BulletType>(torchcraft, "bullettypes"));
  setEnum(py::class_<BW::WeaponType>(torchcraft, "weapontypes"));
  setEnum(py::class_<BW::UnitSize>(torchcraft, "unitsizes"));
  setEnum(py::class_<BW::DamageType>(torchcraft, "dmgtypes"));
  setEnum(py::class_<BW::Color>(torchcraft, "colors"));
}
