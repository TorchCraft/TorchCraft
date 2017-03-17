/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <algorithm>

#include "constants.h"

namespace {

template <typename T>
std::unordered_map<torchcraft::BW::UnitType, int> buildTotalPriceMap(
    const T& prices) {
  std::unordered_map<torchcraft::BW::UnitType, int> total;

  // Production prices for producers
  for (auto ut : torchcraft::BW::UnitType::_values()) {
    if (!torchcraft::BW::unitProductions(ut).empty()) {
      total[ut] = prices[ut];
    }
  }

  // Two separate loops required so that total prices are not overwritten
  for (auto producer : torchcraft::BW::UnitType::_values()) {
    for (auto ut : torchcraft::BW::unitProductions(producer)) {
      int price;
      if (torchcraft::BW::isBuilding(producer) ||
          producer == +torchcraft::BW::UnitType::Zerg_Larva) {
        price = prices[ut];
      } else if (
          ut == +torchcraft::BW::UnitType::Protoss_Archon ||
          ut == +torchcraft::BW::UnitType::Protoss_Dark_Archon) {
        price = 2 * prices[producer];
      } else {
        price = prices[producer] + prices[ut];
      }
      total[ut] = price;
    }
  }

  return total;
}

} // namespace

namespace torchcraft {
namespace BW {

// Returns a sorted (by integral value) vector of units that a given unit is
// able to produce.
std::vector<UnitType> unitProductions(UnitType id) {
  switch (id) {
    case UnitType::Terran_Vulture:
      return {UnitType::Terran_Vulture_Spider_Mine};
    case UnitType::Terran_SCV:
      return {UnitType::Terran_Command_Center,
              UnitType::Terran_Comsat_Station,
              UnitType::Terran_Nuclear_Silo,
              UnitType::Terran_Supply_Depot,
              UnitType::Terran_Refinery,
              UnitType::Terran_Barracks,
              UnitType::Terran_Academy,
              UnitType::Terran_Factory,
              UnitType::Terran_Starport,
              UnitType::Terran_Control_Tower,
              UnitType::Terran_Science_Facility,
              UnitType::Terran_Covert_Ops,
              UnitType::Terran_Physics_Lab,
              UnitType::Terran_Machine_Shop,
              UnitType::Terran_Engineering_Bay,
              UnitType::Terran_Armory,
              UnitType::Terran_Missile_Turret,
              UnitType::Terran_Bunker};
    case UnitType::Zerg_Larva:
      return {UnitType::Zerg_Zergling,
              UnitType::Zerg_Hydralisk,
              UnitType::Zerg_Ultralisk,
              UnitType::Zerg_Drone,
              UnitType::Zerg_Overlord,
              UnitType::Zerg_Mutalisk,
              UnitType::Zerg_Queen,
              UnitType::Zerg_Defiler,
              UnitType::Zerg_Scourge};
    case UnitType::Zerg_Queen:
      return {UnitType::Zerg_Broodling, UnitType::Zerg_Infested_Command_Center};
    case UnitType::Zerg_Hydralisk:
      return {UnitType::Zerg_Lurker};
    case UnitType::Zerg_Drone:
      return {UnitType::Zerg_Hatchery,
              UnitType::Zerg_Lair,
              UnitType::Zerg_Hive,
              UnitType::Zerg_Nydus_Canal,
              UnitType::Zerg_Hydralisk_Den,
              UnitType::Zerg_Defiler_Mound,
              UnitType::Zerg_Greater_Spire,
              UnitType::Zerg_Queens_Nest,
              UnitType::Zerg_Evolution_Chamber,
              UnitType::Zerg_Ultralisk_Cavern,
              UnitType::Zerg_Spire,
              UnitType::Zerg_Spawning_Pool,
              UnitType::Zerg_Creep_Colony,
              UnitType::Zerg_Spore_Colony,
              UnitType::Zerg_Sunken_Colony,
              UnitType::Zerg_Extractor};
    case UnitType::Zerg_Mutalisk:
      return {UnitType::Zerg_Guardian, UnitType::Zerg_Devourer};
    case UnitType::Protoss_Probe:
      return {UnitType::Protoss_Nexus,
              UnitType::Protoss_Robotics_Facility,
              UnitType::Protoss_Pylon,
              UnitType::Protoss_Assimilator,
              UnitType::Protoss_Observatory,
              UnitType::Protoss_Gateway,
              UnitType::Protoss_Photon_Cannon,
              UnitType::Protoss_Citadel_of_Adun,
              UnitType::Protoss_Cybernetics_Core,
              UnitType::Protoss_Templar_Archives,
              UnitType::Protoss_Forge,
              UnitType::Protoss_Stargate,
              UnitType::Protoss_Fleet_Beacon,
              UnitType::Protoss_Arbiter_Tribunal,
              UnitType::Protoss_Robotics_Support_Bay,
              UnitType::Protoss_Shield_Battery};
    case UnitType::Protoss_Carrier:
      return {UnitType::Protoss_Interceptor};
    case UnitType::Protoss_Reaver:
      return {UnitType::Protoss_Scarab};
    case UnitType::Terran_Nuclear_Silo:
      return {UnitType::Terran_Nuclear_Missile};
    case UnitType::Terran_Command_Center:
      return {UnitType::Terran_SCV};
    case UnitType::Terran_Barracks:
      return {UnitType::Terran_Marine,
              UnitType::Terran_Firebat,
              UnitType::Terran_Medic};
    case UnitType::Terran_Factory:
      return {UnitType::Terran_Vulture,
              UnitType::Terran_Siege_Tank_Tank_Mode,
              UnitType::Terran_Goliath};
    case UnitType::Terran_Starport:
      return {UnitType::Terran_Wraith,
              UnitType::Terran_Valkyrie,
              UnitType::Terran_Science_Vessel,
              UnitType::Terran_Dropship,
              UnitType::Terran_Battlecruiser};
    case UnitType::Zerg_Infested_Command_Center:
      return {UnitType::Zerg_Infested_Terran};
    case UnitType::Zerg_Hatchery:
      return {UnitType::Zerg_Larva, UnitType::Zerg_Lair};
    case UnitType::Zerg_Lair:
      return {UnitType::Zerg_Larva, UnitType::Zerg_Hive};
    case UnitType::Zerg_Hive:
      return {UnitType::Zerg_Larva};
    case UnitType::Zerg_Creep_Colony:
      return {UnitType::Zerg_Spore_Colony, UnitType::Zerg_Sunken_Colony};
    case UnitType::Protoss_Nexus:
      return {UnitType::Protoss_Probe};
    case UnitType::Protoss_Robotics_Facility:
      return {UnitType::Protoss_Reaver,
              UnitType::Protoss_Observer,
              UnitType::Protoss_Shuttle};
    case UnitType::Protoss_High_Templar:
      return {UnitType::Protoss_Archon};
    case UnitType::Protoss_Dark_Templar:
      return {UnitType::Protoss_Dark_Archon};
    case UnitType::Protoss_Gateway:
      return {UnitType::Protoss_Zealot,
              UnitType::Protoss_Dragoon,
              UnitType::Protoss_High_Templar,
              UnitType::Protoss_Dark_Templar};
    case UnitType::Protoss_Stargate:
      return {UnitType::Protoss_Scout,
              UnitType::Protoss_Corsair,
              UnitType::Protoss_Carrier};
    default:
      break;
  }
  return {};
}

bool unitProduces(UnitType id, UnitType product) {
  auto prods = unitProductions(id);
  return std::binary_search(prods.begin(), prods.end(), product);
}

std::vector<Order> commandToOrders(UnitCommandType id) {
  // corresponds to BWAPI::UnitCommandTypes to BWAPI::Orders
  switch (id) {
    case UnitCommandType::Halt_Construction:
      return {Order::ResetCollision};
    case UnitCommandType::Upgrade:
      return {Order::Upgrade};
    case UnitCommandType::Cancel_Morph:
      return {Order::PlayerGuard, Order::ResetCollision};
    case UnitCommandType::Return_Cargo:
      return {Order::ReturnGas, Order::ReturnMinerals, Order::ResetCollision};
    case UnitCommandType::Attack_Unit:
      return {Order::AttackUnit, Order::InterceptorAttack, Order::ScarabAttack};
    case UnitCommandType::Cloak:
      return {Order::Cloak};
    case UnitCommandType::Research:
      return {Order::ResearchTech};
    case UnitCommandType::Attack_Move:
      return {Order::AttackMove};
    case UnitCommandType::Build:
      return {Order::PlaceBuilding,
              Order::BuildNydusExit,
              Order::CreateProtossBuilding};
    case UnitCommandType::Right_Click_Unit:
      return {Order::MoveToMinerals,
              Order::MoveToGas,
              Order::ConstructingBuilding,
              Order::AttackUnit,
              Order::Follow,
              Order::ResetCollision,
              Order::EnterNydusCanal,
              Order::EnterTransport,
              Order::Harvest1,
              Order::Harvest2,
              Order::Harvest3,
              Order::Harvest4,
              Order::InterceptorAttack,
              Order::HarvestGas,
              Order::MedicHeal,
              Order::MiningMinerals,
              Order::ReturnMinerals,
              Order::ReturnGas,
              Order::RightClickAction,
              Order::ScarabAttack,
              Order::WaitForGas,
              Order::WaitForMinerals};
    case UnitCommandType::Cancel_Upgrade:
      return {Order::Nothing};
    case UnitCommandType::Siege:
      return {Order::Sieging};
    case UnitCommandType::Train:
      return {Order::Train, Order::TrainFighter};
    case UnitCommandType::Unload:
      return {Order::Unload};
    case UnitCommandType::Stop:
      return {Order::Stop, Order::Interrupted};
    case UnitCommandType::Cancel_Research:
      return {Order::Nothing};
    case UnitCommandType::Lift:
      return {Order::BuildingLiftOff};
    case UnitCommandType::Unburrow:
      return {Order::Unburrowing};
    case UnitCommandType::Cancel_Train_Slot:
      return {Order::Nothing};
    case UnitCommandType::Land:
      return {Order::BuildingLand};
    case UnitCommandType::Set_Rally_Unit:
      return {Order::RallyPointUnit};
    case UnitCommandType::Hold_Position:
      return {Order::HoldPosition};
    case UnitCommandType::Morph:
      return {Order::ZergUnitMorph, Order::ZergBuildingMorph};
    case UnitCommandType::Cancel_Construction:
      return {Order::ResetCollision, Order::Die};
    case UnitCommandType::Gather:
      return {Order::MoveToMinerals,
              Order::MoveToGas,
              Order::Harvest1,
              Order::Harvest2,
              Order::Harvest3,
              Order::Harvest4,
              Order::HarvestGas,
              Order::MiningMinerals,
              Order::WaitForGas,
              Order::WaitForMinerals,
              Order::ResetCollision,
              Order::ReturnMinerals};
    case UnitCommandType::Cancel_Addon:
      return {Order::Nothing};
    case UnitCommandType::Cancel_Train:
      return {Order::Nothing};
    case UnitCommandType::Burrow:
      return {Order::Burrowing};
    case UnitCommandType::Decloak:
      return {Order::Decloak};
    case UnitCommandType::Unsiege:
      return {Order::Unsieging};
    case UnitCommandType::Right_Click_Position:
      return {Order::Move};
    case UnitCommandType::Unload_All:
      return {Order::Unload, Order::MoveUnload};
    case UnitCommandType::Load:
      return {Order::PickupBunker,
              Order::PickupTransport,
              Order::EnterTransport,
              Order::Pickup4};
    case UnitCommandType::Repair:
      return {Order::Repair};
    case UnitCommandType::Unload_All_Position:
      return {Order::MoveUnload};
    case UnitCommandType::Patrol:
      return {Order::Patrol};
    case UnitCommandType::Move:
      return {Order::Move};
    case UnitCommandType::Build_Addon:
      return {Order::BuildAddon, Order::PlaceAddon};
    case UnitCommandType::Set_Rally_Position:
      return {Order::RallyPointTile, Order::RallyPointUnit};
    case UnitCommandType::Follow:
      return {Order::Follow};
    case UnitCommandType::Use_Tech:
      return {Order::Cloak, Order::Decloak};
    case UnitCommandType::Use_Tech_Position:
      return {Order::CastDarkSwarm,
              Order::CastDisruptionWeb,
              Order::CastEMPShockwave,
              Order::CastEnsnare,
              Order::CastNuclearStrike,
              Order::CastRecall,
              Order::CastPsionicStorm,
              Order::CastPlague,
              Order::CastScannerSweep,
              Order::CastStasisField,
              Order::PlaceMine};
    case UnitCommandType::Use_Tech_Unit:
      return {Order::ArchonWarp,
              Order::CastConsume,
              Order::CastDefensiveMatrix,
              Order::CastFeedback,
              Order::CastHallucination,
              Order::CastIrradiate,
              Order::CastInfestation,
              Order::CastLockdown,
              Order::CastMaelstrom,
              Order::CastMindControl,
              Order::CastOpticalFlare,
              Order::CastParasite,
              Order::CastRestoration,
              Order::CastSpawnBroodlings,
              Order::DarkArchonMeld,
              Order::FireYamatoGun,
              Order::InfestingCommandCenter,
              Order::RechargeShieldsUnit};
    default:
      break;
  }
  return {};
}

namespace data {

std::unordered_map<UnitType, int> TotalMineralPrice;
std::unordered_map<UnitType, int> TotalGasPrice;

void init() {
  // Build TotalPrice maps
  TotalMineralPrice = buildTotalPriceMap(MineralPrice);
  TotalGasPrice = buildTotalPriceMap(GasPrice);
}

} // namespace data
} // namespace BW
} // namespace torchcraft
