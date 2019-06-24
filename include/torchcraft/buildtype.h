/*
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>

#include <torchcraft/client.h>

namespace BWAPI {
class UnitType;
class TechType;
class UpgradeType;
} // namespace BWAPI

namespace torchcraft {

/**
 * Represents and holds information about buildable types (units, upgrades,
 * techs).
 *
 * Only one instance exists of a given type, so pointer comparisons can be
 * made. For upgrades, each level is treated as their own unique type.
 * The buildtypes namespace has all of the useful types.
 *
 * Many of these properties directly reference BWAPI properties. See:
 * https://bwapi.github.io/class_b_w_a_p_i_1_1_unit_type.html
 * https://bwapi.github.io/class_b_w_a_p_i_1_1_upgrade_type.html
 * https://bwapi.github.io/class_b_w_a_p_i_1_1_tech_type.html
 */
struct BuildType {
  int unit = -1;
  int upgrade = -1;
  int tech = -1;
  const BuildType* builder = nullptr;
  double mineralCost = 0.0;
  double gasCost = 0.0;
  std::vector<const BuildType*> prerequisites;
  std::string name;
  int buildTime = 0;

  std::vector<const BuildType*> whatUses;
  int level = 0;
  const BuildType* prev = nullptr;

  /**
   * See https://bwapi.github.io/namespace_b_w_a_p_i_1_1_races.html
   * Zerg = 0
   * Terran = 1
   * Protoss = 2
   */
  int race = 0;
  bool isAddon = false;
  bool isWorker = false;
  /**
   * Amount by which this unit increases its race's supply cap.
   * In human-visible Brood War units, as opposed to BWAPI units
   * (ie. a Marine is 1 instead of 2)
   */
  double supplyProvided = 0.0;

  /**
   * Amount by which this unit consumes its race's supply cap.
   * In human-visible Brood War units, as opposed to BWAPI units
   * (ie. a Marine is 1 instead of 2)
   */
  double supplyRequired = 0.0;
  bool isTwoUnitsInOneEgg = false;
  bool isRefinery = false;
  bool isMinerals = false;
  bool isGas = false;
  bool requiresPsi = false;
  bool requiresCreep = false;
  bool isBuilding = false;
  bool isResourceDepot = false;
  bool isResourceContainer = false;
  bool isSpecialBuilding = false;
  int dimensionLeft = 0;
  int dimensionUp = 0;
  int dimensionRight = 0;
  int dimensionDown = 0;
  int tileWidth = 0;
  int tileHeight = 0;
  bool hasAirWeapon = false;
  bool hasGroundWeapon = false;
  int numAirAttacks = 0;
  int numGroundAttacks = 0;
  bool canProduce = false;
  bool canBuildAddon = false;
  bool isFlyer = false;
  bool isDetector = false;
  /// Sight range in walktiles.
  int sightRange = 0;
  bool isNonUsable = false;
  double gScore = 0;
  double subjectiveValue = 0;
  int maxHp = 0;
  int maxShields = 0;
  int maxEnergy = 0;
  int airWeaponCooldown = 0;
  int groundWeaponCooldown = 0;
  int size = 0;
  bool isBiological = false;
  bool producesCreep = false;
  bool producesLarva = false;
  bool restrictedByDarkSwarm = false;

  bool isUnit() const {
    return unit != -1;
  }
  bool isUpgrade() const {
    return upgrade != -1;
  }
  bool isTech() const {
    return tech != -1;
  }
  bool isTerran() const {
    return race == 1;
  }
  bool isProtoss() const {
    return race == 2;
  }
  bool isZerg() const {
    return race == 0;
  }

  BuildType() {}
};

const BuildType* getBuildType(BWAPI::UnitType type);
const BuildType* getUnitBuildType(int unit);
const BuildType* getBuildType(BWAPI::TechType type);
const BuildType* getTechBuildType(int tech);
const BuildType* getBuildType(BWAPI::UpgradeType type, int level = 1);
const BuildType* getUpgradeBuildType(int upgrade, int level = 1);

namespace buildtypes {

extern std::vector<const BuildType*> allUnitTypes;
extern std::vector<const BuildType*> allUpgradeTypes;
extern std::vector<const BuildType*> allTechTypes;

extern const BuildType* Terran_Marine;
extern const BuildType* Terran_Ghost;
extern const BuildType* Terran_Vulture;
extern const BuildType* Terran_Goliath;
extern const BuildType* Terran_Siege_Tank_Tank_Mode;
extern const BuildType* Terran_SCV;
extern const BuildType* Terran_Wraith;
extern const BuildType* Terran_Science_Vessel;
extern const BuildType* Terran_Dropship;
extern const BuildType* Terran_Battlecruiser;
extern const BuildType* Terran_Vulture_Spider_Mine;
extern const BuildType* Terran_Nuclear_Missile;
extern const BuildType* Terran_Civilian;
extern const BuildType* Terran_Siege_Tank_Siege_Mode;
extern const BuildType* Terran_Firebat;
extern const BuildType* Spell_Scanner_Sweep;
extern const BuildType* Terran_Medic;
extern const BuildType* Zerg_Larva;
extern const BuildType* Zerg_Egg;
extern const BuildType* Zerg_Zergling;
extern const BuildType* Zerg_Hydralisk;
extern const BuildType* Zerg_Ultralisk;
extern const BuildType* Zerg_Broodling;
extern const BuildType* Zerg_Drone;
extern const BuildType* Zerg_Overlord;
extern const BuildType* Zerg_Mutalisk;
extern const BuildType* Zerg_Guardian;
extern const BuildType* Zerg_Queen;
extern const BuildType* Zerg_Defiler;
extern const BuildType* Zerg_Scourge;
extern const BuildType* Zerg_Infested_Terran;
extern const BuildType* Terran_Valkyrie;
extern const BuildType* Zerg_Cocoon;
extern const BuildType* Protoss_Corsair;
extern const BuildType* Protoss_Dark_Templar;
extern const BuildType* Zerg_Devourer;
extern const BuildType* Protoss_Dark_Archon;
extern const BuildType* Protoss_Probe;
extern const BuildType* Protoss_Zealot;
extern const BuildType* Protoss_Dragoon;
extern const BuildType* Protoss_High_Templar;
extern const BuildType* Protoss_Archon;
extern const BuildType* Protoss_Shuttle;
extern const BuildType* Protoss_Scout;
extern const BuildType* Protoss_Arbiter;
extern const BuildType* Protoss_Carrier;
extern const BuildType* Protoss_Interceptor;
extern const BuildType* Protoss_Reaver;
extern const BuildType* Protoss_Observer;
extern const BuildType* Protoss_Scarab;
extern const BuildType* Critter_Rhynadon;
extern const BuildType* Critter_Bengalaas;
extern const BuildType* Critter_Scantid;
extern const BuildType* Critter_Kakaru;
extern const BuildType* Critter_Ragnasaur;
extern const BuildType* Critter_Ursadon;
extern const BuildType* Zerg_Lurker_Egg;
extern const BuildType* Zerg_Lurker;
extern const BuildType* Spell_Disruption_Web;
extern const BuildType* Terran_Command_Center;
extern const BuildType* Terran_Comsat_Station;
extern const BuildType* Terran_Nuclear_Silo;
extern const BuildType* Terran_Supply_Depot;
extern const BuildType* Terran_Refinery;
extern const BuildType* Terran_Barracks;
extern const BuildType* Terran_Academy;
extern const BuildType* Terran_Factory;
extern const BuildType* Terran_Starport;
extern const BuildType* Terran_Control_Tower;
extern const BuildType* Terran_Science_Facility;
extern const BuildType* Terran_Covert_Ops;
extern const BuildType* Terran_Physics_Lab;
extern const BuildType* Terran_Machine_Shop;
extern const BuildType* Terran_Engineering_Bay;
extern const BuildType* Terran_Armory;
extern const BuildType* Terran_Missile_Turret;
extern const BuildType* Terran_Bunker;
extern const BuildType* Zerg_Infested_Command_Center;
extern const BuildType* Zerg_Hatchery;
extern const BuildType* Zerg_Lair;
extern const BuildType* Zerg_Hive;
extern const BuildType* Zerg_Nydus_Canal;
extern const BuildType* Zerg_Hydralisk_Den;
extern const BuildType* Zerg_Defiler_Mound;
extern const BuildType* Zerg_Greater_Spire;
extern const BuildType* Zerg_Queens_Nest;
extern const BuildType* Zerg_Evolution_Chamber;
extern const BuildType* Zerg_Ultralisk_Cavern;
extern const BuildType* Zerg_Spire;
extern const BuildType* Zerg_Spawning_Pool;
extern const BuildType* Zerg_Creep_Colony;
extern const BuildType* Zerg_Spore_Colony;
extern const BuildType* Zerg_Sunken_Colony;
extern const BuildType* Zerg_Extractor;
extern const BuildType* Protoss_Nexus;
extern const BuildType* Protoss_Robotics_Facility;
extern const BuildType* Protoss_Pylon;
extern const BuildType* Protoss_Assimilator;
extern const BuildType* Protoss_Observatory;
extern const BuildType* Protoss_Gateway;
extern const BuildType* Protoss_Photon_Cannon;
extern const BuildType* Protoss_Citadel_of_Adun;
extern const BuildType* Protoss_Cybernetics_Core;
extern const BuildType* Protoss_Templar_Archives;
extern const BuildType* Protoss_Forge;
extern const BuildType* Protoss_Stargate;
extern const BuildType* Protoss_Fleet_Beacon;
extern const BuildType* Protoss_Arbiter_Tribunal;
extern const BuildType* Protoss_Robotics_Support_Bay;
extern const BuildType* Protoss_Shield_Battery;
extern const BuildType* Resource_Mineral_Field;
extern const BuildType* Resource_Mineral_Field_Type_2;
extern const BuildType* Resource_Mineral_Field_Type_3;
extern const BuildType* Resource_Vespene_Geyser;
extern const BuildType* Spell_Dark_Swarm;
extern const BuildType* Special_Pit_Door;
extern const BuildType* Special_Right_Pit_Door;

extern const BuildType* Terran_Infantry_Armor_1;
extern const BuildType* Terran_Infantry_Armor_2;
extern const BuildType* Terran_Infantry_Armor_3;
extern const BuildType* Terran_Vehicle_Plating_1;
extern const BuildType* Terran_Vehicle_Plating_2;
extern const BuildType* Terran_Vehicle_Plating_3;
extern const BuildType* Terran_Ship_Plating_1;
extern const BuildType* Terran_Ship_Plating_2;
extern const BuildType* Terran_Ship_Plating_3;
extern const BuildType* Zerg_Carapace_1;
extern const BuildType* Zerg_Carapace_2;
extern const BuildType* Zerg_Carapace_3;
extern const BuildType* Zerg_Flyer_Carapace_1;
extern const BuildType* Zerg_Flyer_Carapace_2;
extern const BuildType* Zerg_Flyer_Carapace_3;
extern const BuildType* Protoss_Ground_Armor_1;
extern const BuildType* Protoss_Ground_Armor_2;
extern const BuildType* Protoss_Ground_Armor_3;
extern const BuildType* Protoss_Air_Armor_1;
extern const BuildType* Protoss_Air_Armor_2;
extern const BuildType* Protoss_Air_Armor_3;
extern const BuildType* Terran_Infantry_Weapons_1;
extern const BuildType* Terran_Infantry_Weapons_2;
extern const BuildType* Terran_Infantry_Weapons_3;
extern const BuildType* Terran_Vehicle_Weapons_1;
extern const BuildType* Terran_Vehicle_Weapons_2;
extern const BuildType* Terran_Vehicle_Weapons_3;
extern const BuildType* Terran_Ship_Weapons_1;
extern const BuildType* Terran_Ship_Weapons_2;
extern const BuildType* Terran_Ship_Weapons_3;
extern const BuildType* Zerg_Melee_Attacks_1;
extern const BuildType* Zerg_Melee_Attacks_2;
extern const BuildType* Zerg_Melee_Attacks_3;
extern const BuildType* Zerg_Missile_Attacks_1;
extern const BuildType* Zerg_Missile_Attacks_2;
extern const BuildType* Zerg_Missile_Attacks_3;
extern const BuildType* Zerg_Flyer_Attacks_1;
extern const BuildType* Zerg_Flyer_Attacks_2;
extern const BuildType* Zerg_Flyer_Attacks_3;
extern const BuildType* Protoss_Ground_Weapons_1;
extern const BuildType* Protoss_Ground_Weapons_2;
extern const BuildType* Protoss_Ground_Weapons_3;
extern const BuildType* Protoss_Air_Weapons_1;
extern const BuildType* Protoss_Air_Weapons_2;
extern const BuildType* Protoss_Air_Weapons_3;
extern const BuildType* Protoss_Plasma_Shields_1;
extern const BuildType* Protoss_Plasma_Shields_2;
extern const BuildType* Protoss_Plasma_Shields_3;
extern const BuildType* U_238_Shells;
extern const BuildType* Ion_Thrusters;
extern const BuildType* Titan_Reactor;
extern const BuildType* Ocular_Implants;
extern const BuildType* Moebius_Reactor;
extern const BuildType* Apollo_Reactor;
extern const BuildType* Colossus_Reactor;
extern const BuildType* Ventral_Sacs;
extern const BuildType* Antennae;
extern const BuildType* Pneumatized_Carapace;
extern const BuildType* Metabolic_Boost;
extern const BuildType* Adrenal_Glands;
extern const BuildType* Muscular_Augments;
extern const BuildType* Grooved_Spines;
extern const BuildType* Gamete_Meiosis;
extern const BuildType* Metasynaptic_Node;
extern const BuildType* Singularity_Charge;
extern const BuildType* Leg_Enhancements;
extern const BuildType* Scarab_Damage;
extern const BuildType* Reaver_Capacity;
extern const BuildType* Gravitic_Drive;
extern const BuildType* Sensor_Array;
extern const BuildType* Gravitic_Boosters;
extern const BuildType* Khaydarin_Amulet;
extern const BuildType* Apial_Sensors;
extern const BuildType* Gravitic_Thrusters;
extern const BuildType* Carrier_Capacity;
extern const BuildType* Khaydarin_Core;
extern const BuildType* Argus_Jewel;
extern const BuildType* Argus_Talisman;
extern const BuildType* Caduceus_Reactor;
extern const BuildType* Chitinous_Plating;
extern const BuildType* Anabolic_Synthesis;
extern const BuildType* Charon_Boosters;

extern const BuildType* Stim_Packs;
extern const BuildType* Lockdown;
extern const BuildType* EMP_Shockwave;
extern const BuildType* Spider_Mines;
extern const BuildType* Scanner_Sweep;
extern const BuildType* Tank_Siege_Mode;
extern const BuildType* Defensive_Matrix;
extern const BuildType* Irradiate;
extern const BuildType* Yamato_Gun;
extern const BuildType* Cloaking_Field;
extern const BuildType* Personnel_Cloaking;
extern const BuildType* Burrowing;
extern const BuildType* Infestation;
extern const BuildType* Spawn_Broodlings;
extern const BuildType* Dark_Swarm;
extern const BuildType* Plague;
extern const BuildType* Consume;
extern const BuildType* Ensnare;
extern const BuildType* Parasite;
extern const BuildType* Psionic_Storm;
extern const BuildType* Hallucination;
extern const BuildType* Recall;
extern const BuildType* Stasis_Field;
extern const BuildType* Archon_Warp;
extern const BuildType* Restoration;
extern const BuildType* Disruption_Web;
extern const BuildType* Mind_Control;
extern const BuildType* Dark_Archon_Meld;
extern const BuildType* Feedback;
extern const BuildType* Optical_Flare;
extern const BuildType* Maelstrom;
extern const BuildType* Lurker_Aspect;
extern const BuildType* Healing;

inline const BuildType* getRaceWorker(int race) {
  switch (race) {
    case torchcraft::BW::Race::Protoss:
      return buildtypes::Protoss_Probe;
    case torchcraft::BW::Race::Zerg:
      return buildtypes::Zerg_Drone;
    default:
      return buildtypes::Terran_SCV;
  }
}
inline const BuildType* getRaceRefinery(int race) {
  switch (race) {
    case torchcraft::BW::Race::Protoss:
      return buildtypes::Protoss_Assimilator;
    case torchcraft::BW::Race::Zerg:
      return buildtypes::Zerg_Extractor;
    default:
      return buildtypes::Terran_Refinery;
  }
}
inline const BuildType* getRaceSupplyDepot(int race) {
  switch (race) {
    case torchcraft::BW::Race::Protoss:
      return buildtypes::Protoss_Pylon;
    case torchcraft::BW::Race::Zerg:
      return buildtypes::Zerg_Overlord;
    default:
      return buildtypes::Terran_Supply_Depot;
  }
}
inline const BuildType* getRaceCommandCenter(int race) {
  switch (race) {
    case torchcraft::BW::Race::Protoss:
      return buildtypes::Protoss_Nexus;
    case torchcraft::BW::Race::Zerg:
      return buildtypes::Zerg_Hatchery;
    default:
      return buildtypes::Terran_Command_Center;
  }
}

void initialize();
} // namespace buildtypes
} // namespace torchcraft
