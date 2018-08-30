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

// This macro maps the member variables of Unit to some IDs
#define _DOALL(F)                     \
  F(x, 0)                             \
  F(y, 1)                             \
  F(health, 2)                        \
  F(max_health, 3)                    \
  F(shield, 4)                        \
  F(max_shield, 5)                    \
  F(energy, 6)                        \
  F(maxCD, 7)                         \
  F(groundCD, 8)                      \
  F(airCD, 9)                         \
  F(visible, 11)                      \
  F(type, 12)                         \
  F(armor, 13)                        \
  F(shieldArmor, 14)                  \
  F(size, 15)                         \
  F(pixel_x, 16)                      \
  F(pixel_y, 17)                      \
  F(pixel_size_x, 18)                 \
  F(pixel_size_y, 19)                 \
  F(groundATK, 20)                    \
  F(airATK, 21)                       \
  F(groundDmgType, 22)                \
  F(airDmgType, 23)                   \
  F(groundRange, 24)                  \
  F(airRange, 25)                     \
  F(playerId, 26)                     \
  F(velocityX, 27)                    \
  F(velocityY, 28)                    \
  F(angle, 29)                        \
  F(resources, 30)                    \
  F(buildTechUpgradeType, 31)         \
  F(remainingBuildTrainTime, 32)      \
  F(remainingUpgradeResearchTime, 33) \
  F(spellCD, 34)                      \
  F(associatedUnit, 35)               \
  F(associatedCount, 36)              \
  F(command.frame, 37)                \
  F(command.type, 38)                 \
  F(command.targetId, 39)             \
  F(command.targetX, 40)              \
  F(command.targetY, 41)              \
  F(command.extra, 42)                

#define _DOALL_ON_ORDER(F) \
  F(first_frame, 0)        \
  F(type, 1)               \
  F(targetId, 2)           \
  F(targetX, 3)            \
  F(targetY, 4)

FrameDiff frame_diff(Frame& lhs, Frame& rhs) {
  return frame_diff(&lhs, &rhs);
}

FrameDiff frame_diff(Frame* lhs, Frame* rhs) {
  FrameDiff df;
  df.latcom_enabled = lhs->latcom_enabled;
  df.remaining_latency_frames = lhs->remaining_latency_frames;
  df.bullets = lhs->bullets;
  df.actions = lhs->actions;
  df.resources = lhs->resources;
  for (size_t i = 0; i < lhs->creep_map.size(); i++) {
    if (lhs->creep_map[i] != rhs->creep_map[i])
      df.creep_map.insert(std::make_pair(i, lhs->creep_map[i]));
  }

  for (auto it : lhs->units) { // Iterates across number of players
    df.pids.push_back(it.first);

    // Set up the units list
    df.units.emplace_back();
    std::vector<detail::UnitDiff>& ul = df.units.back();

    auto lhsu = it.second;
    auto rhsu = rhs->units.find(it.first) == rhs->units.end()
        ? std::vector<Unit>()
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
      du.angle = lit.angle;
      du.flags = lit.flags;
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

Frame* detail::add(Frame* frame, FrameDiff* df) {
  auto f = new Frame();
  detail::add(f, frame, df);
  return f;
}

void detail::add(Frame* f, Frame* frame, FrameDiff* df) {
  f->latcom_enabled = df->latcom_enabled;
  f->remaining_latency_frames = df->remaining_latency_frames;
  f->bullets = df->bullets;
  f->actions = df->actions;
  f->resources = df->resources;
  f->height = frame->height;
  f->width = frame->width;
  f->creep_map = frame->creep_map;
  for (auto pair : df->creep_map)
    f->creep_map[pair.first] = pair.second;

  // We only save units if f and frame are the same pointer
  std::unordered_map<int32_t, std::vector<Unit>> saved_units;
  if (f == frame) {
    saved_units = std::move(frame->units);
  }
  auto& frame_units = (f == frame) ? saved_units : frame->units;
  f->units.clear();

  for (size_t i = 0; i < df->pids.size(); i++) {
    auto pid = df->pids[i];
    f->units[pid] = std::vector<Unit>();

    auto f_units = frame_units.find(pid) == frame_units.end()
        ? std::vector<Unit>()
        : frame_units.at(pid);
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

      Unit& u = f->units[pid].back();
      u.id = du.id;
      u.velocityX = du.velocityX;
      u.velocityY = du.velocityY;
      u.angle = du.angle;
      u.flags = du.flags;

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
}

Frame* frame_undiff(FrameDiff* lhs, Frame* rhs) {
  return detail::add(rhs, lhs);
}

Frame* frame_undiff(Frame* lhs, FrameDiff* rhs) {
  return detail::add(lhs, rhs);
}

void frame_undiff(Frame* frame, FrameDiff* lhs, Frame* rhs) {
  return detail::add(frame, rhs, lhs);
}

void frame_undiff(Frame* frame, Frame* lhs, FrameDiff* rhs) {
  return detail::add(frame, lhs, rhs);
}

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)
#define _TESTMSG(COND, MSG)          \
  if (!(COND)) {                     \
    if (debug)                       \
      std::cerr << MSG << std::endl; \
    return false;                    \
  } else
#define _TEST(COND) _TESTMSG(COND, "(" STRINGIFY(COND) ") not satisfied")
#define _EQV(VAR, CODE) (f1##VAR) CODE == (f2##VAR)CODE
#define _EQ(CODE) (f1) CODE == (f2)CODE
bool detail::frameEq(Frame* f1, Frame* f2, bool debug) {
  _TEST(_EQ(->latcom_enabled));
  _TEST(_EQ(->remaining_latency_frames));
  _TEST(_EQ(->height));
  _TEST(_EQ(->width));
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
    _TEST(_EQ(->resources[i].upgrades));
    _TEST(_EQ(->resources[i].upgrades_level));
    _TEST(_EQ(->resources[i].techs));
  }
  _TEST(f1->creep_map.size() == f2->creep_map.size());
  for (size_t i = 0; i < f1->creep_map.size(); i++)
    _TEST(_EQ(->creep_map[i]));
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
      _TEST(_EQV(units, [i].angle));
      _TEST(_EQV(units, [i].flags));
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


} // namespace replayerr
} // namespace torchcraft
