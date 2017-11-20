/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <algorithm>

#include "frame.h"

namespace torchcraft {
namespace replayer { 

std::ostream& operator<<(std::ostream& out, const Unit& o) {
  out << o.id << " " << o.x << " " << o.y << " " << o.health << " "
      << o.max_health << " " << o.shield << " " << o.max_shield << " "
      << o.energy << " " << o.maxCD << " " << o.groundCD << " " << o.airCD
      << " " << o.flags << " " << o.visible << " " << o.type << " " << o.armor
      << " " << o.shieldArmor << " " << o.size << " " << o.pixel_x << " "
      << o.pixel_y << " " << o.pixel_size_x << " " << o.pixel_size_y << " "
      << o.groundATK << " " << o.airATK << " " << o.groundDmgType << " "
      << o.airDmgType << " " << o.groundRange << " " << o.airRange << " ";

  out << o.orders.size() << " ";
  for (auto& c : o.orders) {
    out << c.first_frame << " " << c.type << " " << c.targetId << " "
        << c.targetX << " " << c.targetY << " ";
  }

  out << o.command.frame << " " << o.command.type << " " << o.command.targetId
      << " " << o.command.targetX << " " << o.command.targetY << " "
      << o.command.extra << " ";

  out << o.velocityX << " " << o.velocityY;
  out << " " << o.playerId;
  out << " " << o.resources;
  out << " " << o.buildTechUpgradeType;
  out << " " << o.remainingBuildTrainTime;
  out << " " << o.remainingUpgradeResearchTime;
  out << " " << o.spellCD;
  out << " " << o.associatedUnit << " " << o.associatedCount;
  return out;
}

std::istream& operator>>(std::istream& in, Unit& o) {
  in >> o.id >> o.x >> o.y >> o.health >> o.max_health >> o.shield >>
      o.max_shield >> o.energy >> o.maxCD >> o.groundCD >> o.airCD >> o.flags >>
      o.visible >> o.type >> o.armor >> o.shieldArmor >> o.size >> o.pixel_x >>
      o.pixel_y >> o.pixel_size_x >> o.pixel_size_y >> o.groundATK >>
      o.airATK >> o.groundDmgType >> o.airDmgType >> o.groundRange >>
      o.airRange;

  int n_orders;
  in >> n_orders;
  if (n_orders < 0)
    throw std::runtime_error("Corrupted replay: n_orders < 0");
  if (n_orders > 10000)
    throw std::runtime_error("Corrupted replay: n_orders > 10000");
  o.orders.resize(n_orders);
  for (int i = 0; i < n_orders; i++) {
    auto& oi = o.orders[i];
    in >> oi.first_frame >> oi.type >> oi.targetId >> oi.targetX >> oi.targetY;
  }

  in >> o.command.frame >> o.command.type >> o.command.targetId >>
      o.command.targetX >> o.command.targetY >> o.command.extra;

  in >> o.velocityX >> o.velocityY;
  in >> o.playerId;
  in >> o.resources;

  in >> o.buildTechUpgradeType;
  in >> o.remainingBuildTrainTime >> o.remainingUpgradeResearchTime;
  in >> o.spellCD >> o.associatedUnit >> o.associatedCount;

  return in;
}

std::ostream& operator<<(
    std::ostream& out,
    const Resources& r) {
  out << r.ore << " " << r.gas << " ";
  out << r.used_psi << " " << r.total_psi << " ";
  out << r.upgrades << " " << r.upgrades_level << " " << r.techs;
  return out;
}

std::istream& operator>>(std::istream& in, Resources& r) {
  in >> r.ore >> r.gas >> r.used_psi >> r.total_psi >> r.upgrades >>
      r.upgrades_level >> r.techs;
  return in;
}

std::ostream& operator<<(
    std::ostream& out,
    const Bullet& o) {
  out << o.type << " " << o.x << " " << o.y;
  return out;
}

std::istream& operator>>(std::istream& in, Bullet& o) {
  in >> o.type >> o.x >> o.y;
  return in;
}

std::ostream& operator<<(
    std::ostream& out,
    const Action& o) {
  out << o.uid << " " << o.aid << " " << o.action.size() << " ";
  for (auto& a : o.action) {
    out << a;
  }
  return out;
}

std::istream& operator>>(std::istream& in, Action& o) {
  in >> o.uid >> o.aid;
  int sizeA;
  in >> sizeA;
  if (sizeA < 0)
    throw std::runtime_error("Corrupted replay: sizeA < 0");

  o.action.resize(sizeA);
  for (int32_t k = 0; k < sizeA; k++) {
    in >> o.action[k];
  }
  return in;
}

// The boolean array better be divisible by 8
std::vector<uint8_t> bool_to_bytes(const std::vector<bool>& arr) {
  std::vector<uint8_t> ret;
  ret.resize(arr.size() / 8);
  for (size_t i = 0; i < arr.size(); i++) {
    ret[i / 8] |= arr[i] << (i % 8);
  }
  return ret;
}
std::vector<bool> bytes_to_bool(const std::vector<uint8_t>& arr) {
  std::vector<bool> ret;
  ret.resize(arr.size() * 8);
  for (size_t i = 0; i < arr.size(); i++) {
    for (size_t k = 0; k < 8; k++) {
      ret[i * 8 + k] = (arr[i] >> k) & 1;
    }
  }
  return ret;
}

std::ostream& operator<<(std::ostream& out, const Frame& o) {

  // Writes the creep map
  out << o.creep_map.size() << " ";
  out.write((const char*)o.creep_map.data(), o.creep_map.size());

  out << o.height << " " << o.width << " ";

  // Writes the Units
  out << o.units.size() << " ";
  for (auto& v : o.units) {
    out << v.first << " " << v.second.size() << " ";
    for (auto& u : v.second) {
      out << u << " ";
    }
  }

  // Writes the rest of frame
  writeTail(out, o.actions, o.resources, o.bullets);

  out << " " << o.reward << " " << o.is_terminal;
  return out;
}

std::istream& operator>>(std::istream& in, Frame& o) {
  int nPlayer, creep_map_size;

  // Read the creep map
  in >> creep_map_size;
  in.ignore(1); // Ignores next space
  o.creep_map.resize(creep_map_size);
  in.read((char*)o.creep_map.data(), creep_map_size);

  in >> o.height >> o.width;

  // Read the units
  in >> nPlayer;
  if (nPlayer < 0)
    throw std::runtime_error("Corrupted replay: units nPlayer < 0");
  if (nPlayer > 9)
    throw std::runtime_error("Corrupted replay: units nPlayer > 9");
  for (int32_t i = 0; i < nPlayer; i++) {
    int idPlayer, nUnits;
    in >> idPlayer >> nUnits;
    if (nUnits < 0)
      throw std::runtime_error("Corrupted replay: nUnits < 0");
    if (nUnits > 10000)
      throw std::runtime_error("Corrupted replay: nUnits > 10000");
    o.units[idPlayer] = std::vector<Unit>();
    o.units[idPlayer].resize(nUnits);
    for (int32_t j = 0; j < nUnits; j++) {
      in >> o.units[idPlayer][j];
    }
  }

  // Read everything else
  readTail(in, o.actions, o.resources, o.bullets);

  in >> o.reward >> o.is_terminal;
  return in;
}

} // namespace replayer
} // namespace torchcraft