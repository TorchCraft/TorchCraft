/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>
#include "lest/lest.hpp"
#include "frame.h"
#include "flatbuffers.h"

namespace torchcraft {
namespace replayer {

  // For comparing properties on test objects
  #define E(property) && a.property == b.property

  bool operator==(const Bullet& a, const Bullet& b) {
    return true E(type) E(x) E(y);
  }

  bool operator==(const Action& a, const Action& b) {
    return true E(action) E(uid) E(aid);
  }
  
  bool operator==(const Resources& a, const Resources& b) {
    return true
      E(ore) E(gas) E(used_psi) E(total_psi)
      E(upgrades) E(upgrades_level) E(techs);
  }

  bool operator==(const Unit& a, const Unit& b) {
    return true
      E(id) E(x) E(y) E(health) E(max_health) E(shield) E(max_shield)
      E(energy) E(maxCD) E(groundCD) E(airCD) E(flags) E(visible) E(type)
      E(armor) E(shieldArmor) E(size) E(pixel_x) E(pixel_y)
      E(pixel_size_x) E(pixel_size_y) E(groundATK) E(airATK)
      E(groundDmgType) E(airDmgType) E(groundRange) E(airRange)
      E(orders) E(command) E(velocityX) E(velocityY)
      E(playerId) E(resources) E(buildTechUpgradeType)
      E(remainingBuildTrainTime) E(remainingUpgradeResearchTime) E(spellCD)
      E(associatedUnit) E(associatedCount);
  }
  
  namespace detail {
    bool operator==(const UnitDiff& a, const UnitDiff& b) {
      return true
      E(id) E(var_ids) E(var_diffs) E(order_ids) E(order_diffs)
        E(order_size) E(velocityX) E(velocityY) E(flags);
    }
  }
  
  template<typename T1, typename T2>
  std::vector<std::pair<T1, T2>> sortMap(std::unordered_map<T1, T2> map) {
    auto comparator = [](
      const std::pair<T1, T2>& a,
      const std::pair<T1, T2>& b) {
      return b.first < a.first;
    };

    std::vector<std::pair<T1, T2>> output(map.begin(), map.end());
    std::sort(output.begin(), output.end(), comparator);
    return output;
  }

  const lest::test specification[] = {
    lest_CASE("A TorchCraft Frame is invariant through serialization") {
      SETUP("Create & serialize a frame") {
        torchcraft::replayer::Frame frameBefore, frameAfter;
        frameBefore.width += 1;
        frameBefore.height += 2;
        frameBefore.reward += 3.4;
        frameBefore.is_terminal = ! frameBefore.is_terminal;
        frameBefore.creep_map = { 1, 2 };
        frameBefore.bullets = {{11, 12, 13}, {21, 22, 23}};
        frameBefore.resources = {
          {11, {12, 13, 14, 15, 16, 17, 18}},
          {21, {22, 23, 24, 25, 26, 27, 28}}};
        frameBefore.actions = {
          {100, {{{111, 112}, 113, 114}, {{121, 122}, 123, 124}}},
          {200, {{{211, 212}, 213, 214}, {{221, 222}, 223, 224}}}};
        frameBefore.units = {
          {1000, {{}, {}}},
          {5000, {{}, {}}}};
        Unit& u1000 = frameBefore.units[1000][0];
        Unit& u2000 = frameBefore.units[1000][1];
        Unit& u5000 = frameBefore.units[5000][0];
        Unit& u6000 = frameBefore.units[5000][1];
        auto propertyId = 1;
        #define U(propertyName) \
          u6000.propertyName = 1000 + ( \
          u5000.propertyName = 3000 + ( \
          u2000.propertyName = 1000 + ( \
          u1000.propertyName = 1000 + propertyId++)));
        U(id) U(x) U(y) U(health) U(max_health) U(shield) U(max_shield)
        U(energy) U(maxCD) U(groundCD) U(airCD) U(flags) U(type)
        U(armor) U(shieldArmor) U(size) U(pixel_x) U(pixel_y)
        U(pixel_size_x) U(pixel_size_y) U(groundATK) U(airATK)
        U(groundDmgType) U(airDmgType) U(groundRange) U(airRange)
        U(velocityX) U(velocityY)
        U(playerId) U(resources) U(buildTechUpgradeType)
        U(remainingBuildTrainTime) U(remainingUpgradeResearchTime) U(spellCD)
        U(associatedUnit) U(associatedCount)
        u5000.visible = u1000.visible = true;
        u6000.visible = u2000.visible = false;
        u1000.orders = {{ 10, 11, 12, 13, 14 }, { -10, -11, -12, -13, -14 }};
        u2000.orders = {{ 20, 21, 22, 23, 24 }, { -20, -21, -22, -23, -24 }};
        u5000.orders = {{ 50, 51, 52, 53, 54 }, { -50, -51, -52, -53, -54 }};
        u6000.orders = {{ 60, 61, 62, 63, 64 }, { -60, -61, -62, -63, -64 }};
        u1000.command = { 101, 102, 103, 104, 105, 106 };
        u2000.command = { 201, 202, 203, 204, 205, 206 };
        u5000.command = { 501, 502, 503, 504, 505, 506 };
        u6000.command = { 601, 602, 603, 604, 605, 606 };
		
        // Our Frame is complete.
        // Let's see how it handles serialization!
        std::stringstream buffer;
        buffer << frameBefore;
        buffer >> frameAfter;

        // Convert unsorted maps to sorted vectors for comparison
        auto sortedResourcesBefore = sortMap(frameBefore.resources);
        auto sortedResourcesAfter = sortMap(frameAfter.resources);
        auto sortedActionsBefore = sortMap(frameBefore.actions);
        auto sortedActionsAfter = sortMap(frameAfter.actions);
        auto sortedUnitsBefore = sortMap(frameBefore.units);
        auto sortedUnitsAfter = sortMap(frameAfter.units);

        // Lest tries to fancy-print EXPECT arguments.
        // But it but doesn't know how to print these complex objects.
        // So we do the comparison here and feed Lest the results.
        auto matchingResources = sortedResourcesBefore == sortedResourcesAfter;
        auto matchingActions = sortedActionsBefore == sortedActionsAfter;
        auto matchingUnits = sortedUnitsBefore == sortedUnitsAfter;

        EXPECT(frameBefore.width == frameAfter.width);
        EXPECT(frameBefore.height == frameAfter.height);
        EXPECT(frameBefore.reward == frameAfter.reward);
        EXPECT(frameBefore.is_terminal == frameAfter.is_terminal);
        EXPECT(frameBefore.creep_map == frameAfter.creep_map);
        EXPECT(frameBefore.bullets == frameAfter.bullets);
        EXPECT(matchingResources);
        EXPECT(matchingActions);
        EXPECT(matchingUnits);
      }
    },
    
    lest_CASE("A TorchCraft FrameDiff is invariant through serialization") {
      SETUP("Create & serialize a FrameDiff") {
        torchcraft::replayer::FrameDiff diffBefore, diffAfter;
        diffBefore.reward = 1.1;
        diffBefore.is_terminal = ! diffBefore.is_terminal;
        diffBefore.pids = {1, 2, 3};
        diffBefore.creep_map = {{11, 12}, {21, 22}};
        diffBefore.bullets = {{11, 12, 13}, {21, 22, 23}};
        diffBefore.resources = {
          {11, {12, 13, 14, 15, 16, 17, 18}},
          {21, {22, 23, 24, 25, 26, 27, 28}}};
        diffBefore.actions = {
          {100, {{{111, 112}, 113, 114}, {{121, 122}, 123, 124}}},
          {200, {{{211, 212}, 213, 214}, {{221, 222}, 223, 224}}}};
        diffBefore.units = {
          {{
            100, {121, 122}, {131, 132}, {141, 142}, {151, 152},
            161, 162.5, 163.5, 164
          }, {
            200, {221, 222}, {231, 232}, {241, 242}, {251, 252},
            261, 262.5, 263.5, 264}},
          {{
            300, {321, 322}, {331, 332}, {341, 342}, {351, 352},
            361, 362.5, 363.5, 364
          }, {
            400, {421, 422}, {431, 432}, {441, 442}, {451, 452},
            461, 462.5, 463.5, 464}}};
            
        std::stringstream buffer;
        buffer << diffBefore;
        buffer >> diffAfter;
        
        auto sortedCreepBefore = sortMap(diffBefore.creep_map);
        auto sortedCreepAfter = sortMap(diffAfter.creep_map);
        auto sortedResourcesBefore = sortMap(diffBefore.resources);
        auto sortedResourcesAfter = sortMap(diffAfter.resources);
        auto sortedActionsBefore = sortMap(diffBefore.actions);
        auto sortedActionsAfter = sortMap(diffAfter.actions);

        // Lest tries to fancy-print EXPECT arguments.
        // But it but doesn't know how to print these complex objects.
        // So we do the comparison here and feed Lest the results.
        auto matchingCreep = sortedCreepBefore == sortedCreepAfter;
        auto matchingResources = sortedResourcesBefore == sortedResourcesAfter;
        auto matchingActions = sortedActionsBefore == sortedActionsAfter;
        auto matchingUnits = diffBefore.units == diffAfter.units;
        
        EXPECT(diffBefore.reward == diffAfter.reward);
        EXPECT(diffBefore.is_terminal == diffAfter.is_terminal);
        EXPECT(diffBefore.pids == diffAfter.pids);
        EXPECT(matchingCreep);
        EXPECT(matchingResources);
        EXPECT(matchingActions);
        EXPECT(matchingUnits);      
      }        
    }
  };
} // namespace torchcraft
} // namespace replayer

// For output, run either as:
// > ctest test --output-on-failure
// > make ARGS='-VV' test
int main(int argc, char* argv[]) {
  std::cout << "Running tests with console output" << std::endl;
  return lest::run(torchcraft::replayer::specification, argc, argv, std::cerr);
}
