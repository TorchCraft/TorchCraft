/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <algorithm>
#include <vector>

#include "messages_generated.h"

namespace torchcraft {
namespace replayer {

// Serialize to FlatBuffers

const auto packActionsOfPlayer = [](flatbuffers::FlatBufferBuilder& builder) {
  return [&builder](const std::pair<int32_t, std::vector<Action>>& actionPair) {
    auto packAction = [&builder](const Action& action) {
      auto actionsOffset = builder.CreateVector(action.action);
      builder.Finish(actionsOffset);

      fbs::ActionBuilder fbsActionBuilder(builder);
      fbsActionBuilder.add_action(actionsOffset);
      fbsActionBuilder.add_uid(action.uid);
      fbsActionBuilder.add_aid(action.aid);
      auto output = fbsActionBuilder.Finish();
      builder.Finish(output);
      return output;
    };

    std::vector<flatbuffers::Offset<fbs::Action>> fbsActions(actionPair.second.size());
    std::transform(actionPair.second.begin(), actionPair.second.end(), fbsActions.begin(), packAction);

    auto actionOffsets = builder.CreateVector(fbsActions);
    builder.Finish(actionOffsets);

    fbs::ActionsOfPlayerBuilder fbsActionsOfPlayerBuilder(builder);
    fbsActionsOfPlayerBuilder.add_playerId(actionPair.first);
    fbsActionsOfPlayerBuilder.add_actions(actionOffsets);
    auto output = fbsActionsOfPlayerBuilder.Finish();
    builder.Finish(output);
    return output;
  };
};

const auto packResourcesOfPlayer = [](flatbuffers::FlatBufferBuilder& builder) {
  return [&builder](const std::pair<int32_t, Resources>& resourcesPair) {
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
    builder.Finish(fbsResources);

    fbs::ResourcesOfPlayerBuilder fbsResourcesOfPlayerBuilder(builder);
    fbsResourcesOfPlayerBuilder.add_playerId(resourcesPair.first);
    fbsResourcesOfPlayerBuilder.add_resources(fbsResources);
    auto output = fbsResourcesOfPlayerBuilder.Finish();
    builder.Finish(output);
    return output;
  };
};

const auto packBullet = [](const Bullet& bullet) {
  return fbs::Bullet(bullet.type, bullet.x, bullet.y);
};

// Deserialize from FlatBuffers

const auto unpackAction = [](const fbs::Action* fbsAction) {
  assert(fbsAction);
  assert(fbsAction->action());
  auto fbsActionInts = fbsAction->action();
  Action action;
  action.action.resize(fbsActionInts->size());
  std::copy(fbsActionInts->begin(), fbsActionInts->end(), action.action.begin());
  action.uid = fbsAction->uid();
  action.aid = fbsAction->aid();
  return action;
};

const auto unpackResources = [](const fbs::ResourcesOfPlayer* fbsResourcesOfPlayer) {
  auto fbsResources = fbsResourcesOfPlayer->resources();
  auto resources = std::make_pair(fbsResourcesOfPlayer->playerId(), Resources()) ;
  resources.second.ore = fbsResources->ore();
  resources.second.gas = fbsResources->gas();
  resources.second.used_psi = fbsResources->used_psi();
  resources.second.total_psi = fbsResources->total_psi();
  resources.second.upgrades = fbsResources->upgrades();
  resources.second.upgrades_level = fbsResources->upgrades_level();
  resources.second.techs = fbsResources->techs();
  return resources;
};

const auto unpackBullet = [](const fbs::Bullet* fbsBullet) {
  Bullet bullet;
  bullet.type = fbsBullet->type();
  bullet.x = fbsBullet->x();
  bullet.y = fbsBullet->y();
  return bullet;
};
   
}; //namespace replayer
}; //namespace storchcraft
