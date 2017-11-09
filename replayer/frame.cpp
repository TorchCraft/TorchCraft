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
#include "flatbuffer_conversions.h"

namespace replayer = torchcraft::replayer;

using Frame = replayer::Frame;

std::ostream& replayer::operator<<(std::ostream& out, const replayer::Frame& frame) {
  flatbuffers::FlatBufferBuilder builder;
  frame.addToFlatBufferBuilder(builder);
  writeFlatBufferToStream(out, builder);
  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Frame& frame) {
  auto flatBufferTable = readFlatBufferTableFromStream<fbs::Frame>(in);
  frame.readFromFlatBufferTable(*flatBufferTable);
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

  std::vector<flatbuffers::Offset<fbs::UnitsByPlayerId>> fbsUnitsByPlayerId;
  std::vector<flatbuffers::Offset<fbs::ActionsByPlayerId>> fbsActionsByPlayerId;
  std::vector<flatbuffers::Offset<fbs::ResourcesByPlayerId>> fbsResourcesByPlayerId;
  std::vector<flatbuffers::Offset<fbs::Bullet>> fbsBullets;
  std::transform(units.begin(), units.end(), fbsUnitsByPlayerId.begin(), buildFbsUnitsByPlayerId);
  std::transform(actions.begin(), actions.end(), fbsActionsByPlayerId.begin(), buildFbsActionsByPlayerId(builder));
  std::transform(resources.begin(), resources.end(), fbsResourcesByPlayerId.begin(), buildFbsResourcesByPlayerId(builder));
  std::transform(bullets.begin(), bullets.end(), fbsBullets.begin(), buildFbsBullet(builder));

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

void Frame::readFromFlatBufferTable(const fbs::Frame& fbsFrame) {

  auto buildUnit = [](const fbs::Unit* fbsUnit) {

    auto buildOrder = [](const fbs::Order* fbsOrder) {
      Order order;
      order.first_frame = fbsOrder->first_frame();
      order.type = fbsOrder->type();
      order.targetId = fbsOrder->targetId();
      order.targetX = fbsOrder->targetX();
      order.targetY = fbsOrder->targetY();
      return order;
    };

    auto fbsOrders = fbsUnit->orders();
    std::vector<Order> orders;
    std::transform(fbsOrders->begin(), fbsOrders->end(), orders.begin(), buildOrder);

    Unit unit;
    auto fbsCommand = fbsUnit->command();
    unit.command.frame = fbsCommand->frame();
    unit.command.type = fbsCommand->type();
    unit.command.targetId = fbsCommand->targetId();
    unit.command.targetX = fbsCommand->targetX();
    unit.command.targetY = fbsCommand->targetY();
    unit.command.extra = fbsCommand->extra();
    unit.id = fbsUnit->id();
    unit.x = fbsUnit->x();
    unit.y = fbsUnit->y();
    unit.health = fbsUnit->health();
    unit.max_health = fbsUnit->max_health();
    unit.shield = fbsUnit->shield();
    unit.max_shield = fbsUnit->max_shield();
    unit.energy = fbsUnit->energy();
    unit.maxCD = fbsUnit->maxCD();
    unit.groundCD = fbsUnit->groundCD();
    unit.airCD = fbsUnit->airCD();
    unit.flags = fbsUnit->flags();
    unit.visible = fbsUnit->visible();
    unit.armor = fbsUnit->armor();
    unit.shieldArmor = fbsUnit->shieldArmor();
    unit.size = fbsUnit->size();
    unit.pixel_x = fbsUnit->pixel_x();
    unit.pixel_y = fbsUnit->pixel_y();
    unit.pixel_size_x = fbsUnit->pixel_size_x();
    unit.pixel_size_y = fbsUnit->pixel_size_y();
    unit.groundATK = fbsUnit->groundATK();
    unit.airATK = fbsUnit->airATK();
    unit.groundDmgType = fbsUnit->groundDmgType();
    unit.airDmgType = fbsUnit->airDmgType();
    unit.groundRange = fbsUnit->groundRange();
    unit.airRange = fbsUnit->airRange();
    unit.velocityX = fbsUnit->velocityX();
    unit.velocityY = fbsUnit->velocityY();
    unit.playerId = fbsUnit->playerId();
    unit.resources = fbsUnit->resources();
    unit.buildTechUpgradeType = fbsUnit->buildTechUpgradeType();
    unit.remainingBuildTrainTime = fbsUnit->remainingBuildTrainTime();
    unit.remainingUpgradeResearchTime = fbsUnit->remainingUpgradeResearchTime();
    unit.spellCD = fbsUnit->spellCD();
    unit.associatedUnit = fbsUnit->associatedUnit();
    unit.associatedCount = fbsUnit->associatedCount();
    unit.orders = orders;
    return unit;
  };

  auto frame = this;
  auto fbsActionsByPlayerIds = fbsFrame.actions();
  auto fbsUnitsByPlayerIds = fbsFrame.units();
  auto fbsResourcesByPlayerIds = fbsFrame.resources();
  auto fbsBullets = fbsFrame.bullets();
  auto fbsCreep = fbsFrame.creep_map();

  std::for_each(
    fbsUnitsByPlayerIds->begin(),
    fbsUnitsByPlayerIds->end(),
    [frame, buildUnit](const fbs::UnitsByPlayerId* fbsUnitsByPlayerId) {
      auto playerId = fbsUnitsByPlayerId->playerId();
      auto fbsUnits = fbsUnitsByPlayerId->units();
      auto units = frame->units[playerId];
      units.clear();
      std::transform(
        fbsUnits->begin(),
        fbsUnits->end(),
        units.begin(),
        buildUnit);
    });

  std::for_each(
    fbsActionsByPlayerIds->begin(),
    fbsActionsByPlayerIds->end(),
    [frame](const fbs::ActionsByPlayerId* fbsActionsByPlayerId) {
      auto playerId = fbsActionsByPlayerId->playerId();
      auto fbsActions = fbsActionsByPlayerId->actions();
      auto actions = frame->actions[playerId];
      actions.clear();
      std::transform(
        fbsActions->begin(),
        fbsActions->end(),
        actions.begin(),
        buildAction);
    });

  resources.clear();
  std::transform(
    fbsResourcesByPlayerIds->begin(),
    fbsResourcesByPlayerIds->end(),
    std::inserter(resources, resources.begin()),
    buildResources);

  bullets.clear();
  std::transform(
    fbsBullets->begin(),
    fbsBullets->end(),
    bullets.begin(),
    buildBullet);

  std::copy(fbsCreep->begin(), fbsCreep->end(), creep_map.begin());

  width = fbsFrame.width();
  height = fbsFrame.height();
  reward = fbsFrame.reward();
  is_terminal = fbsFrame.is_terminal();
}
