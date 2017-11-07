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

using Frame = replayer::Frame;
using FrameDiff = replayer::FrameDiff;

std::ostream& replayer::operator<<(std::ostream& out, const replayer::Frame& frame) {
  flatbuffers::FlatBufferBuilder builder;
  frame.addToFlatBufferBuilder(builder);
  OutStreamableFlatBuffer streamable(builder);
  streamable.write(out);
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Frame& frame) {
  InStreamableFlatBuffer<const fbs::Frame> streamable;
  streamable.read(in);
  auto fbsFrame = streamable.flatBufferTable;
  frame.readFromFlatBufferTable(*fbsFrame);
  return in;
}


std::ostream& replayer::operator<<(std::ostream& out, const FrameDiff& frameDiff) {
  flatbuffers::FlatBufferBuilder builder;
  //frameDiff.addToFlatBufferBuilder(builder);
  OutStreamableFlatBuffer streamable(builder);
  streamable.write(out);
  return out;
}

std::istream& replayer::operator>>(std::istream& in, FrameDiff& frameDiff) {
  InStreamableFlatBuffer<const fbs::Frame> streamable;
  streamable.read(in);
  auto fbsFrame = streamable.flatBufferTable;
  //frameDiff.readFromFlatBufferTable(fbsFrame);
  return in;
}

void Frame::addToFlatBufferBuilder(flatbuffers::FlatBufferBuilder& builder) const {
  auto buildFbsUnitsByPlayerId = [&builder](const std::pair<int32_t, std::vector<Unit>>& unitPair) {
    auto buildFbsUnit = [&builder](const Unit& unit) {
      auto buildFbsOrder = [&builder](const Order& order) {
        fbs::OrderBuilder fbsOrderBuilder(builder);
        fbsOrderBuilder.add_first_frame(order.first_frame);
        fbsOrderBuilder.add_type(order.type);
        fbsOrderBuilder.add_targetId(order.targetId);
        fbsOrderBuilder.add_targetX(order.targetX);
        fbsOrderBuilder.add_targetY(order.targetY);
        return fbsOrderBuilder.Finish();
      };
      std::vector<flatbuffers::Offset<fbs::Order>> fbsOrders;
      std::transform(unit.orders.begin(), unit.orders.end(), fbsOrders.begin(), buildFbsOrder);

      auto command = unit.command;
      fbs::UnitCommandBuilder fbsUnitCommandBuilder(builder);
      fbsUnitCommandBuilder.add_frame(command.frame);
      fbsUnitCommandBuilder.add_type(command.type);
      fbsUnitCommandBuilder.add_targetId(command.targetId);
      fbsUnitCommandBuilder.add_targetX(command.targetX);
      fbsUnitCommandBuilder.add_targetY(command.targetY);
      fbsUnitCommandBuilder.add_extra(command.extra);
      auto fbsCommand = fbsUnitCommandBuilder.Finish();

      fbs::UnitBuilder fbsUnitBuilder(builder);
      fbsUnitBuilder.add_id(unit.id);
      fbsUnitBuilder.add_x(unit.x);
      fbsUnitBuilder.add_y(unit.y);
      fbsUnitBuilder.add_health(unit.health);
      fbsUnitBuilder.add_max_health(unit.max_health);
      fbsUnitBuilder.add_shield(unit.shield);
      fbsUnitBuilder.add_max_shield(unit.max_shield);
      fbsUnitBuilder.add_energy(unit.energy);
      fbsUnitBuilder.add_maxCD(unit.maxCD);
      fbsUnitBuilder.add_groundCD(unit.groundCD);
      fbsUnitBuilder.add_airCD(unit.airCD);
      fbsUnitBuilder.add_flags(unit.flags);
      fbsUnitBuilder.add_visible(unit.visible);
      fbsUnitBuilder.add_armor(unit.armor);
      fbsUnitBuilder.add_shieldArmor(unit.shieldArmor);
      fbsUnitBuilder.add_size(unit.size);
      fbsUnitBuilder.add_pixel_x(unit.pixel_x);
      fbsUnitBuilder.add_pixel_y(unit.pixel_y);
      fbsUnitBuilder.add_pixel_size_x(unit.pixel_size_x);
      fbsUnitBuilder.add_pixel_size_y(unit.pixel_size_y);
      fbsUnitBuilder.add_groundATK(unit.groundATK);
      fbsUnitBuilder.add_airATK(unit.airATK);
      fbsUnitBuilder.add_groundDmgType(unit.groundDmgType);
      fbsUnitBuilder.add_airDmgType(unit.airDmgType);
      fbsUnitBuilder.add_groundRange(unit.groundRange);
      fbsUnitBuilder.add_airRange(unit.airRange);
      fbsUnitBuilder.add_velocityX(unit.velocityX);
      fbsUnitBuilder.add_velocityY(unit.velocityY);
      fbsUnitBuilder.add_playerId(unit.playerId);
      fbsUnitBuilder.add_resources(unit.resources);
      fbsUnitBuilder.add_buildTechUpgradeType(unit.buildTechUpgradeType);
      fbsUnitBuilder.add_remainingBuildTrainTime(unit.remainingBuildTrainTime);
      fbsUnitBuilder.add_remainingUpgradeResearchTime(unit.remainingUpgradeResearchTime);
      fbsUnitBuilder.add_spellCD(unit.spellCD);
      fbsUnitBuilder.add_associatedUnit(unit.associatedUnit);
      fbsUnitBuilder.add_associatedCount(unit.associatedCount);
      fbsUnitBuilder.add_command(fbsCommand);
      fbsUnitBuilder.add_orders(builder.CreateVector(fbsOrders));
      return fbsUnitBuilder.Finish();
    };
    std::vector<flatbuffers::Offset<fbs::Unit>> fbsUnits;
    std::transform(unitPair.second.begin(), unitPair.second.end(), fbsUnits.begin(), buildFbsUnit);
    fbs::UnitsByPlayerIdBuilder fbsUnitsByPlayerIdBuilder(builder);
    fbsUnitsByPlayerIdBuilder.add_playerId(unitPair.first);
    fbsUnitsByPlayerIdBuilder.add_units(builder.CreateVector(fbsUnits));
    return fbsUnitsByPlayerIdBuilder.Finish();
  };
  auto buildFbsActionsByPlayerId = [&builder](const std::pair<int32_t, std::vector<Action>>& actionPair) {
    auto buildFbsAction = [&builder](const Action& action) {
      fbs::ActionBuilder fbsActionBuilder(builder);
      fbsActionBuilder.add_action(builder.CreateVector(action.action));
      fbsActionBuilder.add_uid(action.uid);
      fbsActionBuilder.add_aid(action.aid);
      return fbsActionBuilder.Finish();
    };
    std::vector<flatbuffers::Offset<fbs::Action>> fbsActions;
    std::transform(actionPair.second.begin(), actionPair.second.end(), fbsActions.begin(), buildFbsAction);
    fbs::ActionsByPlayerIdBuilder fbsActionsByPlayerIdBuilder(builder);
    fbsActionsByPlayerIdBuilder.add_playerId(actionPair.first);
    fbsActionsByPlayerIdBuilder.add_actions(builder.CreateVector(fbsActions));
    return fbsActionsByPlayerIdBuilder.Finish();
  };
  auto buildFbsResourcesByPlayerId = [&builder](const std::pair<int32_t, Resources>& resourcesPair) {
    auto resources = resourcesPair.second;
    fbs::ResourcesBuilder fbsResourcesBuilder(builder);
    fbsResourcesBuilder.add_ore(resources.ore);
    fbsResourcesBuilder.add_gas(resources.gas);
    fbsResourcesBuilder.add_used_psi(resources.used_psi);
    fbsResourcesBuilder.add_total_psi(resources.total_psi);
    fbsResourcesBuilder.add_upgrades(resources.upgrades);
    fbsResourcesBuilder.add_upgrades_level(resources.upgrades_level);
    fbsResourcesBuilder.add_techs(resources.techs);
    auto fbsResources = fbsResourcesBuilder.Finish();
    fbs::ResourcesByPlayerIdBuilder fbsResourcesByPlayerIdBuilder(builder);
    fbsResourcesByPlayerIdBuilder.add_playerId(resourcesPair.first);
    fbsResourcesByPlayerIdBuilder.add_resources(fbsResources);
    return fbsResourcesByPlayerIdBuilder.Finish();
  };
  auto buildFbsBullet = [&builder](const Bullet& bullet) {
    fbs::BulletBuilder fbsBulletBuilder(builder);
    fbsBulletBuilder.add_type(bullet.type);
    fbsBulletBuilder.add_x(bullet.x);
    fbsBulletBuilder.add_y(bullet.y);
    return fbsBulletBuilder.Finish();
  };
  std::vector<flatbuffers::Offset<fbs::Bullet>> fbsBullets;
  std::vector<flatbuffers::Offset<fbs::ActionsByPlayerId>> fbsActionsByPlayerId;
  std::vector<flatbuffers::Offset<fbs::UnitsByPlayerId>> fbsUnitsByPlayerId;
  std::vector<flatbuffers::Offset<fbs::ResourcesByPlayerId>> fbsResourcesByPlayerId;
  std::transform(bullets.begin(), bullets.end(), fbsBullets.begin(), buildFbsBullet);
  std::transform(actions.begin(), actions.end(), fbsActionsByPlayerId.begin(), buildFbsActionsByPlayerId);
  std::transform(resources.begin(), resources.end(), fbsResourcesByPlayerId.begin(), buildFbsResourcesByPlayerId);
  std::transform(units.begin(), units.end(), fbsUnitsByPlayerId.begin(), buildFbsUnitsByPlayerId);
  fbs::FrameBuilder fbsFrameBuilder(builder);
  fbsFrameBuilder.add_width(width);
  fbsFrameBuilder.add_height(height);
  fbsFrameBuilder.add_reward(reward);
  fbsFrameBuilder.add_is_terminal(is_terminal);
  fbsFrameBuilder.add_creep_map(builder.CreateVector(creep_map));
  fbsFrameBuilder.add_bullets(builder.CreateVector(fbsBullets));
  fbsFrameBuilder.add_actions(builder.CreateVector(fbsActionsByPlayerId));
  fbsFrameBuilder.add_units(builder.CreateVector(fbsUnitsByPlayerId));
  fbsFrameBuilder.add_resources(builder.CreateVector(fbsResourcesByPlayerId));
  fbsFrameBuilder.Finish();
};

void Frame::readFromFlatBufferTable(const fbs::Frame& table) {
}

namespace detail = replayer::detail;
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
  F(resources, 27)                    \
  F(buildTechUpgradeType, 28)         \
  F(remainingBuildTrainTime, 29)      \
  F(remainingUpgradeResearchTime, 30) \
  F(spellCD, 31)                      \
  F(associatedUnit, 32)               \
  F(associatedCount, 33)              \
  F(command.frame, 34)                \
  F(command.type, 35)                 \
  F(command.targetId, 36)             \
  F(command.targetX, 37)              \
  F(command.targetY, 38)              \
  F(command.extra, 39)

#define _DOALL_ON_ORDER(F) \
  F(first_frame, 0)        \
  F(type, 1)               \
  F(targetId, 2)           \
  F(targetX, 3)            \
  F(targetY, 4)

FrameDiff replayer::frame_diff(Frame& lhs, Frame& rhs) {
  return replayer::frame_diff(&lhs, &rhs);
}

FrameDiff replayer::frame_diff(Frame* lhs, Frame* rhs) {
  FrameDiff df;
  df.reward = lhs->reward;
  df.is_terminal = lhs->is_terminal;
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
  f->reward = df->reward;
  f->is_terminal = df->is_terminal;
  f->bullets = df->bullets;
  f->actions = df->actions;
  f->resources = df->resources;
  f->height = frame->height;
  f->width = frame->width;
  f->creep_map = frame->creep_map;
  for (auto pair : df->creep_map)
    f->creep_map[pair.first] = pair.second;

  f->units.clear();

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

Frame* replayer::frame_undiff(FrameDiff* lhs, Frame* rhs) {
  return detail::add(rhs, lhs);
}

Frame* replayer::frame_undiff(Frame* lhs, FrameDiff* rhs) {
  return detail::add(lhs, rhs);
}

void replayer::frame_undiff(Frame* frame, FrameDiff* lhs, Frame* rhs) {
  return detail::add(frame, rhs, lhs);
}

void replayer::frame_undiff(Frame* frame, Frame* lhs, FrameDiff* rhs) {
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
  _TEST(_EQ(->reward));
  _TEST(_EQ(->is_terminal));
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
