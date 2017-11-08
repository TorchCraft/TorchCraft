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

  auto buildAction = [](const fbs::Action* fbsAction) {
    Action action;
    auto fbsActionInts = fbsAction->action();
    std::copy(fbsActionInts->begin(), fbsActionInts->end(), action.action.begin());
    action.uid = fbsAction->uid();
    action.aid = fbsAction->aid();
    return action;
  };

  auto buildResources = [](const fbs::ResourcesByPlayerId* fbsResourcesByPlayerId) {
    auto fbsResources = fbsResourcesByPlayerId->resources();
    auto resources = std::make_pair(fbsResourcesByPlayerId->playerId(), Resources()) ;
    resources.second.ore = fbsResources->ore();
    resources.second.gas = fbsResources->gas();
    resources.second.used_psi = fbsResources->used_psi();
    resources.second.total_psi = fbsResources->total_psi();
    resources.second.upgrades = fbsResources->upgrades();
    resources.second.upgrades_level = fbsResources->upgrades_level();
    resources.second.techs = fbsResources->techs();
    return resources;
  };

  auto buildBullet = [](const fbs::Bullet* fbsBullet) {
    Bullet bullet;
    bullet.type = fbsBullet->type();
    bullet.x = fbsBullet->x();
    bullet.y = fbsBullet->y();
    return bullet;
  };

  auto frame = this;
  auto fbsActionsByPlayerIds = fbsFrame.actions();
  auto fbsUnitsByPlayerIds = fbsFrame.units();
  auto fbsResourcesByPlyerIds = fbsFrame.resources();
  auto fbsResources = fbsFrame.resources();
  auto fbsBullets = fbsFrame.bullets();
  auto fbsCreep = fbsFrame.creep_map();

  std::for_each(
    fbsUnitsByPlayerIds->begin(),
    fbsUnitsByPlayerIds->end(),
    [frame, buildUnit](const fbs::UnitsByPlayerId* fbsUnitsByPlayerId) {
      auto playerId = fbsUnitsByPlayerId->playerId();
      auto fbsUnits = fbsUnitsByPlayerId->units();
      auto units = frame->units[playerId];
      std::transform(
        fbsUnits->begin(),
        fbsUnits->end(),
        units.begin(),
        buildUnit);
    });

  std::for_each(
    fbsActionsByPlayerIds->begin(),
    fbsActionsByPlayerIds->end(),
    [frame, buildAction](const fbs::ActionsByPlayerId* fbsActionsByPlayerId) {
      auto playerId = fbsActionsByPlayerId->playerId();
      auto fbsActions = fbsActionsByPlayerId->actions();
      auto actions = frame->actions[playerId];
      std::transform(
        fbsActions->begin(),
        fbsActions->end(),
        actions.begin(),
        buildAction);
    });

  std::transform(
    fbsResources->begin(),
    fbsResources->end(),
    std::inserter(resources, resources.end()),
    buildResources);

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
