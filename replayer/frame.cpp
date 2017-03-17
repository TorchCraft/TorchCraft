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

namespace replayer = torchcraft::replayer;

std::ostream& replayer::operator<<(std::ostream& out, const replayer::Unit& o) {
  out << o.id << " " << o.x << " " << o.y << " " << o.health << " "
      << o.max_health << " " << o.shield << " " << o.max_shield << " "
      << o.energy << " " << o.maxCD << " " << o.groundCD << " " << o.airCD
      << " " << o.idle << " " << o.detected << " " << o.lifted << " "
      << o.visible << " " << o.type << " " << o.armor << " " << o.shieldArmor
      << " " << o.size << " " << o.pixel_x << " " << o.pixel_y << " "
      << o.pixel_size_x << " " << o.pixel_size_y << " " << o.groundATK << " "
      << o.airATK << " " << o.groundDmgType << " " << o.airDmgType << " "
      << o.groundRange << " " << o.airRange << " ";

  out << o.orders.size() << " ";
  for (auto& c : o.orders) {
    out << c.first_frame << " " << c.type << " " << c.targetId << " "
        << c.targetX << " " << c.targetY << " ";
  }

  out << o.velocityX << " " << o.velocityY;
  out << " " << o.playerId;
  out << " " << o.resources;
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Unit& o) {
  in >> o.id >> o.x >> o.y >> o.health >> o.max_health >> o.shield >>
      o.max_shield >> o.energy >> o.maxCD >> o.groundCD >> o.airCD >> o.idle >>
      o.detected >> o.lifted >> o.visible >> o.type >> o.armor >>
      o.shieldArmor >> o.size >> o.pixel_x >> o.pixel_y >> o.pixel_size_x >>
      o.pixel_size_y >> o.groundATK >> o.airATK >> o.groundDmgType >>
      o.airDmgType >> o.groundRange >> o.airRange;

  int n_orders;
  in >> n_orders;
  if (n_orders < 0)
    throw std::runtime_error("Corrupted replay: n_orders < 0");
  o.orders.resize(n_orders);
  for (int i = 0; i < n_orders; i++) {
    in >> o.orders[i].first_frame >> o.orders[i].type >> o.orders[i].targetId >>
        o.orders[i].targetX >> o.orders[i].targetY;
  }

  in >> o.velocityX >> o.velocityY;
  in >> o.playerId;
  in >> o.resources;
  return in;
}

std::ostream& replayer::operator<<(
    std::ostream& out,
    const replayer::Resources& r) {
  out << r.ore << " " << r.gas << " ";
  out << r.used_psi << " " << r.total_psi;
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Resources& r) {
  in >> r.ore >> r.gas >> r.used_psi >> r.total_psi;
  return in;
}

std::ostream& replayer::operator<<(
    std::ostream& out,
    const replayer::Bullet& o) {
  out << o.type << " " << o.x << " " << o.y;
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Bullet& o) {
  in >> o.type >> o.x >> o.y;
  return in;
}

std::ostream& replayer::operator<<(
    std::ostream& out,
    const replayer::Action& o) {
  out << o.uid << " " << o.aid << " " << o.action.size() << " ";
  for (auto& a : o.action) {
    out << a;
  }
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Action& o) {
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

void writeTail(
    std::ostream& out,
    const std::unordered_map<int32_t, std::vector<replayer::Action>>& actions,
    const std::unordered_map<int32_t, replayer::Resources>& resources,
    const std::vector<replayer::Bullet>& bullets) {
  out << actions.size() << " ";
  for (auto& v : actions) {
    out << v.first << " " << v.second.size() << " ";
    for (auto& u : v.second) {
      out << u << " ";
    }
  }
  out << resources.size() << " ";
  for (auto& r : resources) {
    out << r.first << " " << r.second << " ";
  }
  out << bullets.size();
  for (auto& b : bullets) {
    out << " " << b;
  }
}

void readTail(
    std::istream& in,
    std::unordered_map<int32_t, std::vector<replayer::Action>>& actions,
    std::unordered_map<int32_t, replayer::Resources>& resources,
    std::vector<replayer::Bullet>& bullets) {
  int nPlayer, nBullets;

  in >> nPlayer;
  if (nPlayer < 0)
    throw std::runtime_error("Corrupted replay: actions nPlayer < 0");
  for (int32_t i = 0; i < nPlayer; i++) {
    int32_t idPlayer, nActions;
    in >> idPlayer >> nActions;
    if (nActions < 0)
      throw std::runtime_error("Corrupted replay: nActions < 0");
    actions[idPlayer] = std::vector<replayer::Action>();
    actions[idPlayer].resize(nActions);
    for (int32_t j = 0; j < nActions; j++) {
      in >> actions[idPlayer][j];
    }
  }

  in >> nPlayer;
  if (nPlayer < 0)
    throw std::runtime_error("Corrupted replay: resources nPlayer < 0");
  for (int32_t i = 0; i < nPlayer; i++) {
    int32_t idPlayer;
    in >> idPlayer;
    in >> resources[idPlayer];
  }

  in >> nBullets;
  if (nBullets < 0)
    throw std::runtime_error("Corrupted replay: nBullets < 0");
  bullets.resize(nBullets);
  for (int32_t i = 0; i < nBullets; i++) {
    in >> bullets[i];
  }
}

std::ostream& replayer::operator<<(
    std::ostream& out,
    const replayer::Frame& o) {
  out << o.units.size() << " ";
  for (auto& v : o.units) {
    out << v.first << " " << v.second.size() << " ";
    for (auto& u : v.second) {
      out << u << " ";
    }
  }

  writeTail(out, o.actions, o.resources, o.bullets);

  out << " " << o.reward << " " << o.is_terminal;
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Frame& o) {
  int nPlayer;

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

  readTail(in, o.actions, o.resources, o.bullets);

  in >> o.reward >> o.is_terminal;
  return in;
}

namespace detail = replayer::detail;
// This macro maps the member variables of Unit to some IDs
#define _DOALL(F)      \
  F(x, 0)              \
  F(y, 1)              \
  F(health, 2)         \
  F(max_health, 3)     \
  F(shield, 4)         \
  F(max_shield, 5)     \
  F(energy, 6)         \
  F(maxCD, 7)          \
  F(groundCD, 8)       \
  F(airCD, 9)          \
  F(idle, 10)          \
  F(visible, 11)       \
  F(type, 12)          \
  F(armor, 13)         \
  F(shieldArmor, 14)   \
  F(size, 15)          \
  F(pixel_x, 16)       \
  F(pixel_y, 17)       \
  F(pixel_size_x, 18)  \
  F(pixel_size_y, 19)  \
  F(groundATK, 20)     \
  F(airATK, 21)        \
  F(groundDmgType, 22) \
  F(airDmgType, 23)    \
  F(groundRange, 24)   \
  F(airRange, 25)      \
  F(playerId, 26)      \
  F(resources, 27)     \
  F(detected, 28)      \
  F(lifted, 29)

#define _DOALL_ON_ORDER(F) \
  F(first_frame, 0)        \
  F(type, 1)               \
  F(targetId, 2)           \
  F(targetX, 3)            \
  F(targetY, 4)

replayer::FrameDiff replayer::frame_diff(
    replayer::Frame& lhs,
    replayer::Frame& rhs) {
  return replayer::frame_diff(&lhs, &rhs);
}

replayer::FrameDiff replayer::frame_diff(
    replayer::Frame* lhs,
    replayer::Frame* rhs) {
  replayer::FrameDiff df;
  df.reward = lhs->reward;
  df.is_terminal = lhs->is_terminal;
  df.bullets = lhs->bullets;
  df.actions = lhs->actions;
  df.resources = lhs->resources;
  for (auto it : lhs->units) { // Iterates across number of players
    df.pids.push_back(it.first);

    // Set up the units list
    df.units.emplace_back();
    std::vector<detail::UnitDiff>& ul = df.units.back();

    auto lhsu = it.second;
    auto rhsu = rhs->units.find(it.first) == rhs->units.end()
        ? std::vector<replayer::Unit>()
        : rhs->units.at(it.first);
    std::sort(lhsu.begin(), lhsu.end(), detail::orderUnitByiD);
    std::sort(rhsu.begin(), rhsu.end(), detail::orderUnitByiD);
    auto rit = rhsu.begin();
    for (auto lit : lhsu) {
      while (rit != rhsu.end() && lit.id > rit->id)
        rit++;
      ul.emplace_back();
      detail::UnitDiff& du = ul.back();
      du.id = lit.id;
      du.velocityX = lit.velocityX;
      du.velocityY = lit.velocityY;
      du.order_size = lit.orders.size();
      if (rit != rhsu.end() &&
          lit.id == rit->id) { // Unit exists in both frames
        int32_t buffer = 0;
// Fill out diffs for the int32_t variables
#define _GEN_VAR(NAME, NUM)         \
  buffer = lit.NAME - rit->NAME;    \
  if (buffer != 0) {                \
    du.var_ids.push_back(NUM);      \
    du.var_diffs.push_back(buffer); \
  }
        _DOALL(_GEN_VAR)
#undef _GEN_VAR
        // Fill out diffs for orders
        for (size_t i = 0; i < lit.orders.size(); i++) {
#define _GEN_VAR(NAME, NUM)                            \
  if (i >= rit->orders.size())                         \
    buffer = lit.orders[i].NAME;                       \
  else                                                 \
    buffer = lit.orders[i].NAME - rit->orders[i].NAME; \
  if (buffer != 0) {                                   \
    du.order_ids.push_back(5 * i + NUM);               \
    du.order_diffs.push_back(buffer);                  \
  }
          _DOALL_ON_ORDER(_GEN_VAR)
#undef _GEN_VAR
        }
      } else { // Unit only exist in latter frame;
// Fill out diffs for the int32_t variables
#define _GEN_VAR(NAME, NUM)  \
  du.var_ids.push_back(NUM); \
  du.var_diffs.push_back(lit.NAME);
        _DOALL(_GEN_VAR)
#undef _GEN_VAR
        // Fill out diffs for orders
        for (size_t i = 0; i < lit.orders.size(); i++) {
#define _GEN_VAR(NAME, NUM)            \
  du.order_ids.push_back(5 * i + NUM); \
  du.order_diffs.push_back(lit.orders[i].NAME);
          _DOALL_ON_ORDER(_GEN_VAR)
#undef _GEN_VAR
        }
      } // end if
    } // end loop over units
  } // end loop over players

  return df;
}

replayer::Frame* detail::add(replayer::Frame* frame, replayer::FrameDiff* df) {
  auto f = new replayer::Frame();
  f->reward = df->reward;
  f->is_terminal = df->is_terminal;
  f->bullets = df->bullets;
  f->actions = df->actions;
  f->resources = df->resources;
  for (size_t i = 0; i < df->pids.size(); i++) {
    auto pid = df->pids[i];
    f->units[pid] = std::vector<replayer::Unit>();

    auto f_units = frame->units.find(pid) == frame->units.end()
        ? std::vector<replayer::Unit>()
        : frame->units.at(pid);
    std::sort(f_units.begin(), f_units.end(), detail::orderUnitByiD);

    // Should be in order
    auto fit = f_units.begin();
    for (auto du : df->units[i]) {
      while (fit != f_units.end() && fit->id < du.id)
        fit++;

      if (fit != f_units.end() && du.id == fit->id)
        f->units[pid].emplace_back(*fit);
      else
        f->units[pid].emplace_back();

      replayer::Unit& u = f->units[pid].back();
      u.id = du.id;
      u.velocityX = du.velocityX;
      u.velocityY = du.velocityY;

      for (size_t k = 0; k < du.var_diffs.size(); k++) {
        switch (du.var_ids[k]) { // assumes int32_t are 0 initted
#define _SWITCHES(NAME, NUM)   \
  case NUM:                    \
    u.NAME += du.var_diffs[k]; \
    break;
          _DOALL(_SWITCHES)
#undef _SWITCHES
        }
      }

      u.orders.resize(du.order_size);
      for (size_t k = 0; k < du.order_diffs.size(); k++) {
        auto order_n = du.order_ids[k] / 5;
        auto field_n = du.order_ids[k] % 5;
        switch (field_n) {
#define _SWITCHES(NAME, NUM)                     \
  case NUM:                                      \
    u.orders[order_n].NAME += du.order_diffs[k]; \
    break;
          _DOALL_ON_ORDER(_SWITCHES)
#undef _SWITCHES
        }
      }
    }
  }

  return f;
}

replayer::Frame* replayer::frame_undiff(
    replayer::FrameDiff& lhs,
    replayer::Frame& rhs) {
  return detail::add(&rhs, &lhs);
}

replayer::Frame* replayer::frame_undiff(
    replayer::Frame& lhs,
    replayer::FrameDiff& rhs) {
  return detail::add(&lhs, &rhs);
}

replayer::Frame* replayer::frame_undiff(
    replayer::FrameDiff* lhs,
    replayer::Frame* rhs) {
  return detail::add(rhs, lhs);
}

replayer::Frame* replayer::frame_undiff(
    replayer::Frame* lhs,
    replayer::FrameDiff* rhs) {
  return detail::add(lhs, rhs);
}

std::ostream& replayer::operator<<(
    std::ostream& out,
    const replayer::FrameDiff& o) {
  out << o.pids.size();
  for (auto& pid : o.pids)
    out << " " << pid;
  for (auto& player_unit : o.units) {
    out << " " << player_unit.size() << " ";
    for (auto& du : player_unit)
      out << du << " ";
  }
  writeTail(out, o.actions, o.resources, o.bullets);
  out << " " << o.reward << " " << o.is_terminal;
  return out;
}

std::ostream& replayer::operator<<(
    std::ostream& out,
    const detail::UnitDiff& o) {
  out << o.id << " " << o.velocityX << " " << o.velocityY;
  out << " " << o.var_ids.size();
  for (size_t i = 0; i < o.var_ids.size(); i++)
    out << " " << o.var_ids[i];
  for (size_t i = 0; i < o.var_ids.size(); i++)
    out << " " << o.var_diffs[i];
  out << " " << o.order_size << " " << o.order_ids.size();
  for (size_t i = 0; i < o.order_ids.size(); i++)
    out << " " << o.order_ids[i];
  for (size_t i = 0; i < o.order_ids.size(); i++)
    out << " " << o.order_diffs[i];
  return out;
}

std::istream& replayer::operator>>(std::istream& in, FrameDiff& o) {
  int32_t npids;
  in >> npids;
  o.pids.resize(npids);
  o.units.resize(npids);
  for (size_t i = 0; i < npids; i++)
    in >> o.pids[i];
  for (size_t i = 0, nunits = 0; i < npids; i++) {
    in >> nunits;
    o.units[i].resize(nunits);
    for (size_t k = 0; k < nunits; k++)
      in >> o.units[i][k];
  }
  readTail(in, o.actions, o.resources, o.bullets);
  in >> o.reward >> o.is_terminal;
  return in;
}

std::istream& replayer::operator>>(std::istream& in, detail::UnitDiff& o) {
  size_t nvars, norders;

  in >> o.id >> o.velocityX >> o.velocityY;
  in >> nvars;
  o.var_ids.resize(nvars);
  o.var_diffs.resize(nvars);
  for (size_t i = 0; i < nvars; i++)
    in >> o.var_ids[i];
  for (size_t i = 0; i < nvars; i++)
    in >> o.var_diffs[i];
  in >> o.order_size >> norders;
  o.order_ids.resize(norders);
  o.order_diffs.resize(norders);
  for (size_t i = 0; i < norders; i++)
    in >> o.order_ids[i];
  for (size_t i = 0; i < norders; i++)
    in >> o.order_diffs[i];
  return in;
}

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)
#define _TESTMSG(COND, MSG)        \
  if (!(COND)) {                   \
    std::cerr << MSG << std::endl; \
    return false;                  \
  } else
#define _TEST(COND) _TESTMSG(COND, "(" STRINGIFY(COND) ") not satisfied")
#define _EQV(VAR, CODE) (f1##VAR) CODE == (f2##VAR)CODE
#define _EQ(CODE) (f1) CODE == (f2)CODE
bool detail::frameEq(replayer::Frame* f1, replayer::Frame* f2) {
  _TEST(_EQ(->reward));
  _TEST(_EQ(->is_terminal));
  _TEST(_EQ(->bullets.size()));
  for (size_t i = 0; i < f1->bullets.size(); i++) {
    _TEST(_EQ(->bullets[i].type));
    _TEST(_EQ(->bullets[i].x));
    _TEST(_EQ(->bullets[i].y));
  }
  _TEST(f1->resources.size() == f2->resources.size());
  for (size_t i = 0; i < f1->resources.size(); i++) {
    _TEST(_EQ(->resources[i].ore));
    _TEST(_EQ(->resources[i].gas));
    _TEST(_EQ(->resources[i].used_psi));
    _TEST(_EQ(->resources[i].total_psi));
  }
  _TEST(_EQ(->actions.size()));
  for (auto elem : f1->actions) {
    _TEST(f2->actions.find(elem.first) != f2->actions.end());
    auto f1actions = elem.second;
    auto f2actions = f2->actions.at(elem.first);
    _TEST(_EQV(actions, .size()));
    for (size_t i = 0; i < f1actions.size(); i++) {
      _TEST(_EQV(actions, [i].uid));
      _TEST(_EQV(actions, [i].aid));
      _TEST(_EQV(actions, [i].action.size()));
      for (size_t k = 0; k < f1actions[i].action.size(); k++)
        _TEST(_EQV(actions, [i].action[k]));
    }
  }
  for (auto elem : f1->units) {
    _TEST(f2->units.find(elem.first) != f2->units.end());
    auto f1units = elem.second;
    auto f2units = f2->units.at(elem.first);
    std::sort(f1units.begin(), f1units.end(), detail::orderUnitByiD);
    std::sort(f2units.begin(), f2units.end(), detail::orderUnitByiD);
    _TEST(_EQV(units, .size()));
    for (size_t i = 0; i < f1units.size(); i++) {
#define _GEN_VAR(NAME, NUM) _TEST(_EQV(units, [i].NAME));
      _DOALL(_GEN_VAR)
#undef _GEN_VAR
      _TEST(_EQV(units, [i].velocityX));
      _TEST(_EQV(units, [i].velocityY));
      _TEST(_EQV(units, [i].orders.size()));
      for (size_t k = 0; k < f1units[i].orders.size(); k++)
        _TEST(_EQV(units, [i].orders[k]));
    }
  }
  return true;
}
#undef _TESTMSG
#undef _TEST
#undef _EQV
#undef _EQ
#undef STRINGIFY2
#undef STRINGIFY

#undef _DOALL
#undef _DOALL_ON_ORDER
