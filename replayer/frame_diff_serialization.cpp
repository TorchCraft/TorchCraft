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
#include "streaming_flatbuffers.h"

namespace torchcraft {
namespace replayer {

std::ostream& operator<<(std::ostream& out, const FrameDiff& frameDiff) {
  flatbuffers::FlatBufferBuilder builder;
  frameDiff.addToFlatBufferBuilder(builder);
  writeFlatBufferToStream(out, builder);
  return out;
}

std::istream& operator>>(std::istream& in, FrameDiff& frameDiff) {
  auto converter = [&frameDiff](const fbs::FrameDiff& fbsFrameDiff) {
    frameDiff.readFromFlatBufferTable(fbsFrameDiff);
  };
  readFlatBufferTableFromStream<fbs::FrameDiff>(in, converter);
  return in;
}

flatbuffers::Offset<fbs::FrameDiff> FrameDiff::addToFlatBufferBuilder(flatbuffers::FlatBufferBuilder& builder) const {

  auto buildFbsFrameDiffCreep = [](const std::pair<int32_t, int32_t> creepPair) {
    return fbs::FrameDiffCreep(creepPair.first, creepPair.second);
  };

  auto buildFbsUnitDiffContainer = [&builder](const std::vector<detail::UnitDiff>& unitDiffs) {
    auto buildFbsUnitDiff = [&builder](const detail::UnitDiff& unitDiff) {
      
      auto var_ids_offsets = builder.CreateVector(unitDiff.var_ids);
      builder.Finish(var_ids_offsets);
      
      auto var_diffs_offsets = builder.CreateVector(unitDiff.var_diffs);
      builder.Finish(var_diffs_offsets);
      
      auto order_ids_offsets = builder.CreateVector(unitDiff.order_ids);
      builder.Finish(order_ids_offsets);
      
      auto order_diffs_offsets = builder.CreateVector(unitDiff.order_diffs);
      builder.Finish(order_diffs_offsets);
      
      fbs::UnitDiffBuilder fbsUnitDiffBuilder(builder);
      fbsUnitDiffBuilder.add_var_ids(var_ids_offsets);
      fbsUnitDiffBuilder.add_var_diffs(var_diffs_offsets);
      fbsUnitDiffBuilder.add_order_ids(order_ids_offsets);
      fbsUnitDiffBuilder.add_order_diffs(order_diffs_offsets);
      fbsUnitDiffBuilder.add_id(unitDiff.id);
      fbsUnitDiffBuilder.add_order_size(unitDiff.order_size);
      fbsUnitDiffBuilder.add_velocityX(unitDiff.velocityX);
      fbsUnitDiffBuilder.add_velocityY(unitDiff.velocityY);
      fbsUnitDiffBuilder.add_flags(unitDiff.flags);
      auto output = fbsUnitDiffBuilder.Finish();
      builder.Finish(output);
      return output;
    };

    std::vector<flatbuffers::Offset<fbs::UnitDiff>> fbsUnitDiffs(unitDiffs.size());
    std::transform(unitDiffs.begin(), unitDiffs.end(), fbsUnitDiffs.begin(), buildFbsUnitDiff);
    auto unitDiffsOffsets = builder.CreateVector(fbsUnitDiffs);
    builder.Finish(unitDiffsOffsets);

    fbs::UnitDiffContainerBuilder fbsUnitDiffContainerBuilder(builder);
    fbsUnitDiffContainerBuilder.add_units(unitDiffsOffsets);
    auto output = fbsUnitDiffContainerBuilder.Finish();
    builder.Finish(output);
    return output;
  };

  std::vector<fbs::FrameDiffCreep> fbsCreep(creep_map.size());
  std::vector<fbs::Bullet> fbsBullets(bullets.size());
  std::vector<flatbuffers::Offset<fbs::ResourcesOfPlayer>> fbsResourcesOfPlayer(resources.size());
  std::vector<flatbuffers::Offset<fbs::ActionsOfPlayer>> fbsActionsOfPlayer(actions.size());  
  std::vector<flatbuffers::Offset<fbs::UnitDiffContainer>> fbsUnitDiffContainers(units.size());

  std::transform(creep_map.begin(), creep_map.end(), fbsCreep.begin(), buildFbsFrameDiffCreep);
  std::transform(bullets.begin(), bullets.end(), fbsBullets.begin(), buildFbsBullet);
  std::transform(resources.begin(), resources.end(), fbsResourcesOfPlayer.begin(), buildFbsResourcesOfPlayer(builder));
  std::transform(actions.begin(), actions.end(), fbsActionsOfPlayer.begin(), buildFbsActionsOfPlayer(builder));   
  std::transform(units.begin(), units.end(), fbsUnitDiffContainers.begin(), buildFbsUnitDiffContainer);

  auto pidsOffsets = builder.CreateVector(pids);
  builder.Finish(pidsOffsets);
  
  auto creepMapOffsets = builder.CreateVectorOfStructs(fbsCreep);
  builder.Finish(creepMapOffsets);
  
  auto bulletsOffsets = builder.CreateVectorOfStructs(fbsBullets);
  builder.Finish(bulletsOffsets);
  
  auto actionsOffsets = builder.CreateVector(fbsActionsOfPlayer);
  builder.Finish(actionsOffsets);
  
  auto resourcesOffsets = builder.CreateVector(fbsResourcesOfPlayer);
  builder.Finish(resourcesOffsets);
  
  auto unitDiffsOffsets = builder.CreateVector(fbsUnitDiffContainers);
  builder.Finish(unitDiffsOffsets);

  fbs::FrameDiffBuilder fbsFrameDiffBuilder(builder);
  fbsFrameDiffBuilder.add_reward(reward);
  fbsFrameDiffBuilder.add_is_terminal(is_terminal);
  fbsFrameDiffBuilder.add_pids(pidsOffsets); 
  fbsFrameDiffBuilder.add_creep_map(creepMapOffsets);
  fbsFrameDiffBuilder.add_bullets(bulletsOffsets);
  fbsFrameDiffBuilder.add_resources(resourcesOffsets);
  fbsFrameDiffBuilder.add_actions(actionsOffsets);
  fbsFrameDiffBuilder.add_unitDiffContainers(unitDiffsOffsets);
  auto output = fbsFrameDiffBuilder.Finish();
  builder.Finish(output);
  return output;
}

void FrameDiff::readFromFlatBufferTable(const fbs::FrameDiff& fbsFrameDiff) {

  auto buildUnits = [](const fbs::UnitDiffContainer* fbsUnitDiffContainer) {
    auto buildUnit = [](const fbs::UnitDiff* fbsUnitDiff) {
      detail::UnitDiff unitDiff;
      auto fbsVarIds = fbsUnitDiff->var_ids();
      auto fbsVarDiffs = fbsUnitDiff->var_diffs();
      auto fbsOrderIds = fbsUnitDiff->order_ids();
      auto fbsOrderDiffs = fbsUnitDiff->order_diffs();
      unitDiff.var_ids.resize(fbsVarIds->size());
      unitDiff.var_diffs.resize(fbsVarDiffs->size());
      unitDiff.order_ids.resize(fbsOrderIds->size());
      unitDiff.order_diffs.resize(fbsOrderDiffs->size());
      std::copy(fbsVarIds->begin(), fbsVarIds->end(), unitDiff.var_ids.begin());
      std::copy(fbsVarDiffs->begin(), fbsVarDiffs->end(), unitDiff.var_diffs.begin());
      std::copy(fbsOrderIds->begin(), fbsOrderIds->end(), unitDiff.order_ids.begin());
      std::copy(fbsOrderDiffs->begin(), fbsOrderDiffs->end(), unitDiff.order_diffs.begin());
      unitDiff.id = fbsUnitDiff->id();
      unitDiff.order_size = fbsUnitDiff->order_size();
      unitDiff.velocityX = fbsUnitDiff->velocityX ();
      unitDiff.velocityY = fbsUnitDiff->velocityY();
      unitDiff.flags = fbsUnitDiff->flags();
      return unitDiff;
    };

    auto fbsUnitDiffs = fbsUnitDiffContainer->units();
    std::vector<detail::UnitDiff> unitDiffs(fbsUnitDiffs->size());
    std::transform(
      fbsUnitDiffs->begin(),
      fbsUnitDiffs->end(),
      unitDiffs.begin(),
      buildUnit);
    return unitDiffs;
  };

  auto buildCreep = [](const fbs::FrameDiffCreep* fbsCreep) {
    return std::make_pair(fbsCreep->index(), fbsCreep->creep());
  };

  auto frameDiff = this;
  auto fbsPids = fbsFrameDiff.pids();
  auto fbsCreep = fbsFrameDiff.creep_map();
  auto fbsBullets = fbsFrameDiff.bullets();
  auto fbsResourcesOfPlayers = fbsFrameDiff.resources();
  auto fbsActionsOfPlayers = fbsFrameDiff.actions();
  auto fbsUnitDiffContainers = fbsFrameDiff.unitDiffContainers();  

  pids.clear();
  pids.resize(fbsPids->size());
  std::copy(fbsPids->begin(), fbsPids->end(), pids.begin());
  
  creep_map.clear();
  std::transform(
    fbsCreep->begin(),
    fbsCreep->end(),
    std::inserter(creep_map, creep_map.begin()),
    buildCreep);
    
    
  bullets.clear();
  bullets.resize(fbsBullets->size());
  std::transform(
    fbsBullets->begin(),
    fbsBullets->end(),
    bullets.begin(),
    buildBullet);
    
  resources.clear();
  std::transform(
    fbsResourcesOfPlayers->begin(),
    fbsResourcesOfPlayers->end(),
    std::inserter(resources, resources.end()),
    buildResources);
    
  actions.clear();
  std::for_each(
    fbsActionsOfPlayers->begin(),
    fbsActionsOfPlayers->end(),
    [frameDiff](const fbs::ActionsOfPlayer* fbsActionsOfPlayer) {
      auto playerId = fbsActionsOfPlayer->playerId();
      auto fbsActions = fbsActionsOfPlayer->actions();
      auto& playerActions = frameDiff->actions[playerId];
      playerActions.clear();
      playerActions.resize(fbsActions->size());
      std::transform(
        fbsActions->begin(),
        fbsActions->end(),
        playerActions.begin(),
        buildAction);
    });
    
  units.clear();
  units.resize(fbsUnitDiffContainers->size());
  std::transform(
    fbsUnitDiffContainers->begin(),
    fbsUnitDiffContainers->end(),
    units.begin(),
    buildUnits);

  reward = fbsFrameDiff.reward();
  is_terminal = fbsFrameDiff.is_terminal();
}

} // namespace replayer
} // namespace torchcraft
