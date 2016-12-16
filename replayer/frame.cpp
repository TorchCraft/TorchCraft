/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "frame.h"

std::ostream& replayer::operator<<(std::ostream& out, const replayer::Unit& o) {
  out << o.id << " " << o.x << " " << o.y << " "
    << o.health << " " << o.max_health << " "
    << o.shield << " " << o.max_shield << " " << o.energy << " " << o.maxCD << " "
    << o.groundCD << " " << o.airCD << " " << o.idle << " "
    << o.visible << " " << o.type << " " << o.armor << " "
    << o.shieldArmor << " " << o.size << " "
    << o.pixel_x << " " << o.pixel_y << " "
    << o.pixel_size_x << " " << o.pixel_size_y << " "
    << o.groundATK << " " << o.airATK << " " << o.groundDmgType << " "
    << o.airDmgType << " " << o.groundRange << " " << o.airRange << " ";

  out << o.orders.size() << " ";
  for (auto& c : o.orders) {
    out << c.first_frame << " "
      << c.type << " " << c.targetId << " "
      << c.targetX << " " << c.targetY << " ";
  }

  out << o.velocityX << " " << o.velocityY;
  out << " " << o.playerId;
  out << " " << o.resources;
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Unit& o) {
  in >> o.id >> o.x >> o.y >> o.health >> o.max_health >> o.shield 
    >> o.max_shield >> o.energy
    >> o.maxCD >> o.groundCD >> o.airCD >> o.idle >> o.visible >> o.type
    >> o.armor >> o.shieldArmor >> o.size
    >> o.pixel_x >> o.pixel_y >> o.pixel_size_x >> o.pixel_size_y
    >> o.groundATK >> o.airATK >> o.groundDmgType >> o.airDmgType
    >> o.groundRange >> o.airRange;

  int n_orders;
  in >> n_orders;
  if (n_orders < 0)
    throw std::runtime_error("Corrupted replay: n_orders < 0");
  o.orders.resize(n_orders);
  for (int i = 0; i < n_orders; i++) {
    in >> o.orders[i].first_frame
      >> o.orders[i].type >> o.orders[i].targetId
      >> o.orders[i].targetX >> o.orders[i].targetY;
  }

  in >> o.velocityX >> o.velocityY;
  in >> o.playerId;
  in >> o.resources;
  return in;
}

std::ostream& replayer::operator<<(std::ostream& out,
  const replayer::Resources& r) {
  out << r.ore << " " << r.gas << " ";
  out << r.used_psi << " " << r.total_psi;
  return out;
}

std::istream& replayer::operator>>(std::istream& in,
  replayer::Resources& r) {
  in >> r.ore >> r.gas >> r.used_psi >> r.total_psi;
  return in;
}

std::ostream& replayer::operator<<(std::ostream& out,
  const replayer::Bullet& o) {
  out << o.type << " " << o.x << " " << o.y;
  return out;
}

std::istream& replayer::operator>>(std::istream& in,
  replayer::Bullet& o) {
  in >> o.type >> o.x >> o.y;
  return in;
}

std::ostream& replayer::operator<<(std::ostream& out,
  const replayer::Frame& o) {
  out << o.units.size() << " ";
  for (auto& v : o.units) {
    out << v.first << " " << v.second.size() << " ";
    for (auto& u : v.second) {
      out << u << " ";
    }
  }
  out << o.actions.size() << " ";
  for (auto& v : o.actions) {
    out << v.first << " " << v.second.size() << " ";
    for (auto& u : v.second) {
      out << u.uid << " " << u.aid << " " << u.action.size() << " ";
      for (auto& a : u.action) {
        out << a << " ";
      }
    }
  }
  out << o.resources.size() << " ";
  for (auto& r : o.resources) {
    out << r.first << " " << r.second << " ";
  }
  out << o.bullets.size() << " ";
  for (auto& b : o.bullets) {
    out << b << " ";
  }
  out << o.reward << " " << o.is_terminal;
  return out;
}

std::istream& replayer::operator>>(std::istream& in,
  replayer::Frame& o) {
  int nPlayer, nBullets;

  in >> nPlayer;
  if (nPlayer < 0)
    throw std::runtime_error("Corrupted replay: units nPlayer < 0");
  for (int32_t i = 0; i < nPlayer; i++) {
    int idPlayer, nUnits;
    in >> idPlayer >> nUnits;
    if (nUnits < 0)
      throw std::runtime_error("Corrupted replay: nUnits < 0");
    o.units[idPlayer] = std::vector<replayer::Unit>();
    o.units[idPlayer].resize(nUnits);
    for (int32_t j = 0; j < nUnits; j++) {
      in >> o.units[idPlayer][j];
    }
  }

  in >> nPlayer;
  if (nPlayer < 0)
    throw std::runtime_error("Corrupted replay: actions nPlayer < 0");
  for (int32_t i = 0; i < nPlayer; i++) {
    int32_t idPlayer, nActions;
    in >> idPlayer >> nActions;
    if (nActions < 0)
      throw std::runtime_error("Corrupted replay: nActions < 0");
    o.actions[idPlayer] = std::vector<replayer::Action>();
    o.actions[idPlayer].resize(nActions);
    for (int32_t j = 0; j < nActions; j++) {
      in >> o.actions[idPlayer][j].uid >> o.actions[idPlayer][j].aid;
      int sizeA;
      in >> sizeA;
      if (sizeA < 0)
        throw std::runtime_error("Corrupted replay: sizeA < 0");

      o.actions[idPlayer][j].action.resize(sizeA);
      for (int32_t k = 0; k < sizeA; k++) {
        in >> o.actions[idPlayer][j].action[k];
      }
    }
  }

  in >> nPlayer;
  if (nPlayer < 0)
    throw std::runtime_error("Corrupted replay: resources nPlayer < 0");
  for (int32_t i = 0; i < nPlayer; i++) {
    int32_t idPlayer;
    in >> idPlayer;
    in >> o.resources[idPlayer];
  }

  in >> nBullets;
  if (nBullets < 0)
    throw std::runtime_error("Corrupted replay: nBullets < 0");
  o.bullets.resize(nBullets);
  for (int32_t i = 0; i < nBullets; i++) {
    in >> o.bullets[i];
  }
  in >> o.reward >> o.is_terminal;
  return in;
}
