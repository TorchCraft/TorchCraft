/*
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <torchcraft/buildtype.h>

#include <BWAPI.h>
#include <algorithm>
#include <array>
#include <string>

namespace torchcraft {

namespace buildtypes {
std::array<BuildType, BWAPI::UnitTypes::Enum::MAX> BuildTypeUnitArray;
std::array<BuildType, BWAPI::TechTypes::Enum::MAX> BuildTypeTechArray;
std::array<BuildType, BWAPI::UpgradeTypes::Enum::MAX * 3> BuildTypeUpgradeArray;

std::vector<const BuildType*> allUnitTypes;
std::vector<const BuildType*> allUpgradeTypes;
std::vector<const BuildType*> allTechTypes;
} // namespace buildtypes
using namespace buildtypes;

static int sumBuildTime(const BuildType* type) {
  std::unordered_set<const BuildType*> visited;
  int r = 0;
  std::function<void(const BuildType*)> visit = [&](const BuildType* n) {
    if (!visited.insert(n).second)
      return;
    r += n->buildTime;
    for (const BuildType* nn : n->prerequisites) {
      visit(nn);
    }
  };
  visit(type);
  return r;
}

const BuildType* getBuildType(BWAPI::UnitType type) {
  int id = type.getID();
  if (id == BWAPI::UnitTypes::Enum::None)
    return nullptr;
  BuildType* r = &BuildTypeUnitArray.at(id);
  if (r->unit == id) {
    return r;
  }
  allUnitTypes.push_back(r);
  r->unit = id;
  r->builder = getBuildType(type.whatBuilds().first);
  r->mineralCost = (double)type.mineralPrice();
  r->gasCost = (double)type.gasPrice();
  for (auto&& v : type.requiredUnits()) {
    r->prerequisites.push_back(getBuildType(v.first));
  }
  if (r->builder) {
    r->prerequisites.push_back(r->builder);
  }
  if (type == BWAPI::UnitTypes::Zerg_Lurker) {
    r->prerequisites.push_back(getBuildType(BWAPI::TechTypes::Lurker_Aspect));
  }
  std::stable_sort(
      r->prerequisites.begin(), r->prerequisites.end(), [](auto* a, auto* b) {
        return sumBuildTime(a) > sumBuildTime(b);
      });
  r->name = type.getName();
  r->buildTime = type.buildTime();

  r->race = type.getRace();
  r->isAddon = type.isAddon();
  r->isWorker = type.isWorker();
  r->supplyProvided = type.supplyProvided() / 2.0;
  r->supplyRequired = type.supplyRequired() / 2.0;
  r->isTwoUnitsInOneEgg = type.isTwoUnitsInOneEgg();
  r->isRefinery = type.isRefinery();
  r->isMinerals = type.isMineralField();
  r->isGas = type == BWAPI::UnitTypes::Resource_Vespene_Geyser || r->isRefinery;
  r->requiresPsi = type.requiresPsi();
  r->requiresCreep = type.requiresCreep();
  r->isBuilding = type.isBuilding();
  r->isResourceDepot = type.isResourceDepot();
  r->isResourceContainer = type.isResourceContainer();
  r->isSpecialBuilding = type.isSpecialBuilding();
  r->dimensionLeft = type.dimensionLeft();
  r->dimensionUp = type.dimensionUp();
  r->dimensionRight = type.dimensionRight();
  r->dimensionDown = type.dimensionDown();
  r->tileWidth = type.tileWidth();
  r->tileHeight = type.tileHeight();
  r->hasAirWeapon = type.airWeapon() != BWAPI::WeaponTypes::None;
  r->hasGroundWeapon = type.groundWeapon() != BWAPI::WeaponTypes::None;
  r->numAirAttacks = r->hasAirWeapon ? 1 : 0;
  r->numGroundAttacks = r->hasGroundWeapon ? 1 : 0;
  r->canProduce = type.canProduce();
  r->canBuildAddon = type.canBuildAddon();
  r->isFlyer = type.isFlyer();
  r->isDetector = type.isDetector();
  r->sightRange = BW::XYWalktilesPerBuildtile * type.sightRange() / 32;
  r->isNonUsable = type == BWAPI::UnitTypes::Zerg_Larva ||
      type == BWAPI::UnitTypes::Zerg_Egg ||
      type == BWAPI::UnitTypes::Zerg_Lurker_Egg ||
      type == BWAPI::UnitTypes::Zerg_Cocoon ||
      type == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine ||
      type == BWAPI::UnitTypes::Terran_Nuclear_Missile ||
      type == BWAPI::UnitTypes::Protoss_Scarab || type.maxHitPoints() == 1;
  r->gScore = r->mineralCost + (4.0 / 3) * r->gasCost + 50 * r->supplyRequired;

  // Some constants for establishing subjective values of various buildtypes.
  // Anchored somewhat in game mechanics
  // (providing 8 supply costs 100 minerals; 6.25 = 8/100)
  // but also somewhat arbitrary (Zergs tend to have less income so their units
  // are proportionally more valuable -- but how much more?)
  constexpr double buildtypeValueMinerals = 1.0;
  constexpr double buildtypeValueGas = 1.3;
  constexpr double buildtypeValueSupply = 6.25;
  constexpr double buildtypeValueTerran = 1.0;
  constexpr double buildtypeValueProtoss = 1.0;
  constexpr double buildtypeValueZerg = 1.2;
  r->subjectiveValue = buildtypeValueMinerals * r->mineralCost;
  r->subjectiveValue += buildtypeValueGas * r->gasCost;
  r->subjectiveValue *= r->isTwoUnitsInOneEgg ? 0.5 : 1.0;
  r->subjectiveValue += buildtypeValueSupply * r->supplyRequired;
  r->subjectiveValue *= r->isTerran() ? buildtypeValueTerran : 1.0;
  r->subjectiveValue *= r->isProtoss() ? buildtypeValueProtoss : 1.0;
  r->subjectiveValue *= r->isZerg() ? buildtypeValueZerg : 1.0;

  r->maxHp = type.maxHitPoints();
  r->maxShields = type.maxShields();
  r->maxEnergy = type.maxEnergy();
  r->airWeaponCooldown =
      r->hasAirWeapon ? type.airWeapon().damageCooldown() : 0;
  r->groundWeaponCooldown =
      r->hasGroundWeapon ? type.groundWeapon().damageCooldown() : 0;
  r->size = (int)type.size();
  r->isBiological = type.isOrganic();
  r->producesCreep = type.producesCreep();
  r->producesLarva = type.producesLarva();
  r->restrictedByDarkSwarm = r->hasGroundWeapon &&
      type != BWAPI::UnitTypes::Terran_Firebat &&
      type != BWAPI::UnitTypes::Protoss_Dark_Templar &&
      type != BWAPI::UnitTypes::Protoss_Reaver &&
      type != BWAPI::UnitTypes::Protoss_Scarab &&
      type != BWAPI::UnitTypes::Protoss_Zealot &&
      type != BWAPI::UnitTypes::Zerg_Broodling &&
      type != BWAPI::UnitTypes::Zerg_Infested_Terran &&
      type != BWAPI::UnitTypes::Zerg_Ultralisk &&
      type != BWAPI::UnitTypes::Zerg_Zergling &&
      type != BWAPI::UnitTypes::Zerg_Lurker;

  switch (id) {
    case BWAPI::UnitTypes::Enum::Protoss_Zealot:
      r->numGroundAttacks = 2;
      break;
    case BWAPI::UnitTypes::Enum::Terran_Firebat:
      r->numGroundAttacks = 3;
      break;
    case BWAPI::UnitTypes::Enum::Terran_Valkyrie:
      r->numAirAttacks = 4;
      break;
    case BWAPI::UnitTypes::Enum::Terran_Goliath:
    case BWAPI::UnitTypes::Enum::Protoss_Scout:
      r->numAirAttacks = 2;
      break;
    case BWAPI::UnitTypes::Enum::Protoss_Carrier:
      r->hasGroundWeapon = true;
      r->hasAirWeapon = true;
      break;
    case BWAPI::UnitTypes::Enum::Protoss_Reaver:
      r->hasGroundWeapon = true;
      break;
    default:
      break;
  }

  return r;
}

const BuildType* getUnitBuildType(int unit) {
  return getBuildType(BWAPI::UnitType(unit));
}

const BuildType* getBuildType(BWAPI::TechType type) {
  int id = type.getID();
  if (id == BWAPI::TechTypes::Enum::None) {
    return nullptr;
  }
  BuildType* r = &BuildTypeTechArray.at(id);
  if (r->tech == id) {
    return r;
  }
  allTechTypes.push_back(r);
  r->tech = id;
  r->builder = getBuildType(type.whatResearches());
  r->mineralCost = (double)type.mineralPrice();
  r->gasCost = (double)type.gasPrice();
  if (type == BWAPI::TechTypes::Lurker_Aspect) {
    r->prerequisites.push_back(getBuildType(BWAPI::UnitTypes::Zerg_Lair));
  }
  if (r->builder) {
    r->prerequisites.push_back(r->builder);
  }
  r->name = type.getName();
  r->buildTime = type.researchTime();
  r->race = type.getRace();
  r->level = 1;
  for (auto& v : type.whatUses()) {
    r->whatUses.push_back(getBuildType(v));
  }
  return r;
}

const BuildType* getTechBuildType(int tech) {
  return getBuildType(BWAPI::TechType(tech));
}

const BuildType* getBuildType(BWAPI::UpgradeType type, int level) {
  int id = type.getID();
  if (id == BWAPI::UpgradeTypes::Enum::None)
    return nullptr;
  BuildType* r = &BuildTypeUpgradeArray.at(
      BWAPI::UpgradeTypes::Enum::MAX * (level - 1) + id);
  if (r->upgrade == id) {
    return r;
  }
  allUpgradeTypes.push_back(r);
  r->upgrade = id;
  r->builder = getBuildType(type.whatUpgrades());
  r->mineralCost = (double)type.mineralPrice();
  r->gasCost = (double)type.gasPrice();
  const BuildType* req = getUnitBuildType(type.whatsRequired(level));
  if (req) {
    r->prerequisites.push_back(req);
  }
  if (r->builder) {
    r->prerequisites.push_back(r->builder);
  }
  r->name = type.getName();
  if (type.maxRepeats() > 1) {
    r->name += "_" + std::to_string(level);
  }
  r->buildTime = type.upgradeTime();
  r->race = type.getRace();
  r->level = level;
  r->prev = level == 1 ? nullptr : getBuildType(type, level - 1);
  if (r->prev) {
    r->prerequisites.push_back(r->prev);
  }
  std::stable_sort(
      r->prerequisites.begin(), r->prerequisites.end(), [](auto* a, auto* b) {
        return sumBuildTime(a) > sumBuildTime(b);
      });
  for (auto& v : type.whatUses()) {
    r->whatUses.push_back(getBuildType(v));
  }
  return r;
}

const BuildType* getUpgradeBuildType(int upgrade, int level) {
  return getBuildType(BWAPI::UpgradeType(upgrade), level);
}

namespace buildtypes {

const BuildType* Terran_Marine;
const BuildType* Terran_Ghost;
const BuildType* Terran_Vulture;
const BuildType* Terran_Goliath;
const BuildType* Terran_Siege_Tank_Tank_Mode;
const BuildType* Terran_SCV;
const BuildType* Terran_Wraith;
const BuildType* Terran_Science_Vessel;
const BuildType* Terran_Dropship;
const BuildType* Terran_Battlecruiser;
const BuildType* Terran_Vulture_Spider_Mine;
const BuildType* Terran_Nuclear_Missile;
const BuildType* Terran_Civilian;
const BuildType* Terran_Siege_Tank_Siege_Mode;
const BuildType* Terran_Firebat;
const BuildType* Spell_Scanner_Sweep;
const BuildType* Terran_Medic;
const BuildType* Zerg_Larva;
const BuildType* Zerg_Egg;
const BuildType* Zerg_Zergling;
const BuildType* Zerg_Hydralisk;
const BuildType* Zerg_Ultralisk;
const BuildType* Zerg_Broodling;
const BuildType* Zerg_Drone;
const BuildType* Zerg_Overlord;
const BuildType* Zerg_Mutalisk;
const BuildType* Zerg_Guardian;
const BuildType* Zerg_Queen;
const BuildType* Zerg_Defiler;
const BuildType* Zerg_Scourge;
const BuildType* Zerg_Infested_Terran;
const BuildType* Terran_Valkyrie;
const BuildType* Zerg_Cocoon;
const BuildType* Protoss_Corsair;
const BuildType* Protoss_Dark_Templar;
const BuildType* Zerg_Devourer;
const BuildType* Protoss_Dark_Archon;
const BuildType* Protoss_Probe;
const BuildType* Protoss_Zealot;
const BuildType* Protoss_Dragoon;
const BuildType* Protoss_High_Templar;
const BuildType* Protoss_Archon;
const BuildType* Protoss_Shuttle;
const BuildType* Protoss_Scout;
const BuildType* Protoss_Arbiter;
const BuildType* Protoss_Carrier;
const BuildType* Protoss_Interceptor;
const BuildType* Protoss_Reaver;
const BuildType* Protoss_Observer;
const BuildType* Protoss_Scarab;
const BuildType* Critter_Rhynadon;
const BuildType* Critter_Bengalaas;
const BuildType* Critter_Scantid;
const BuildType* Critter_Kakaru;
const BuildType* Critter_Ragnasaur;
const BuildType* Critter_Ursadon;
const BuildType* Zerg_Lurker_Egg;
const BuildType* Zerg_Lurker;
const BuildType* Spell_Disruption_Web;
const BuildType* Terran_Command_Center;
const BuildType* Terran_Comsat_Station;
const BuildType* Terran_Nuclear_Silo;
const BuildType* Terran_Supply_Depot;
const BuildType* Terran_Refinery;
const BuildType* Terran_Barracks;
const BuildType* Terran_Academy;
const BuildType* Terran_Factory;
const BuildType* Terran_Starport;
const BuildType* Terran_Control_Tower;
const BuildType* Terran_Science_Facility;
const BuildType* Terran_Covert_Ops;
const BuildType* Terran_Physics_Lab;
const BuildType* Terran_Machine_Shop;
const BuildType* Terran_Engineering_Bay;
const BuildType* Terran_Armory;
const BuildType* Terran_Missile_Turret;
const BuildType* Terran_Bunker;
const BuildType* Zerg_Infested_Command_Center;
const BuildType* Zerg_Hatchery;
const BuildType* Zerg_Lair;
const BuildType* Zerg_Hive;
const BuildType* Zerg_Nydus_Canal;
const BuildType* Zerg_Hydralisk_Den;
const BuildType* Zerg_Defiler_Mound;
const BuildType* Zerg_Greater_Spire;
const BuildType* Zerg_Queens_Nest;
const BuildType* Zerg_Evolution_Chamber;
const BuildType* Zerg_Ultralisk_Cavern;
const BuildType* Zerg_Spire;
const BuildType* Zerg_Spawning_Pool;
const BuildType* Zerg_Creep_Colony;
const BuildType* Zerg_Spore_Colony;
const BuildType* Zerg_Sunken_Colony;
const BuildType* Zerg_Extractor;
const BuildType* Protoss_Nexus;
const BuildType* Protoss_Robotics_Facility;
const BuildType* Protoss_Pylon;
const BuildType* Protoss_Assimilator;
const BuildType* Protoss_Observatory;
const BuildType* Protoss_Gateway;
const BuildType* Protoss_Photon_Cannon;
const BuildType* Protoss_Citadel_of_Adun;
const BuildType* Protoss_Cybernetics_Core;
const BuildType* Protoss_Templar_Archives;
const BuildType* Protoss_Forge;
const BuildType* Protoss_Stargate;
const BuildType* Protoss_Fleet_Beacon;
const BuildType* Protoss_Arbiter_Tribunal;
const BuildType* Protoss_Robotics_Support_Bay;
const BuildType* Protoss_Shield_Battery;
const BuildType* Resource_Mineral_Field;
const BuildType* Resource_Mineral_Field_Type_2;
const BuildType* Resource_Mineral_Field_Type_3;
const BuildType* Resource_Vespene_Geyser;
const BuildType* Spell_Dark_Swarm;
const BuildType* Special_Pit_Door;
const BuildType* Special_Right_Pit_Door;

const BuildType* Terran_Infantry_Armor_1;
const BuildType* Terran_Infantry_Armor_2;
const BuildType* Terran_Infantry_Armor_3;
const BuildType* Terran_Vehicle_Plating_1;
const BuildType* Terran_Vehicle_Plating_2;
const BuildType* Terran_Vehicle_Plating_3;
const BuildType* Terran_Ship_Plating_1;
const BuildType* Terran_Ship_Plating_2;
const BuildType* Terran_Ship_Plating_3;
const BuildType* Zerg_Carapace_1;
const BuildType* Zerg_Carapace_2;
const BuildType* Zerg_Carapace_3;
const BuildType* Zerg_Flyer_Carapace_1;
const BuildType* Zerg_Flyer_Carapace_2;
const BuildType* Zerg_Flyer_Carapace_3;
const BuildType* Protoss_Ground_Armor_1;
const BuildType* Protoss_Ground_Armor_2;
const BuildType* Protoss_Ground_Armor_3;
const BuildType* Protoss_Air_Armor_1;
const BuildType* Protoss_Air_Armor_2;
const BuildType* Protoss_Air_Armor_3;
const BuildType* Terran_Infantry_Weapons_1;
const BuildType* Terran_Infantry_Weapons_2;
const BuildType* Terran_Infantry_Weapons_3;
const BuildType* Terran_Vehicle_Weapons_1;
const BuildType* Terran_Vehicle_Weapons_2;
const BuildType* Terran_Vehicle_Weapons_3;
const BuildType* Terran_Ship_Weapons_1;
const BuildType* Terran_Ship_Weapons_2;
const BuildType* Terran_Ship_Weapons_3;
const BuildType* Zerg_Melee_Attacks_1;
const BuildType* Zerg_Melee_Attacks_2;
const BuildType* Zerg_Melee_Attacks_3;
const BuildType* Zerg_Missile_Attacks_1;
const BuildType* Zerg_Missile_Attacks_2;
const BuildType* Zerg_Missile_Attacks_3;
const BuildType* Zerg_Flyer_Attacks_1;
const BuildType* Zerg_Flyer_Attacks_2;
const BuildType* Zerg_Flyer_Attacks_3;
const BuildType* Protoss_Ground_Weapons_1;
const BuildType* Protoss_Ground_Weapons_2;
const BuildType* Protoss_Ground_Weapons_3;
const BuildType* Protoss_Air_Weapons_1;
const BuildType* Protoss_Air_Weapons_2;
const BuildType* Protoss_Air_Weapons_3;
const BuildType* Protoss_Plasma_Shields_1;
const BuildType* Protoss_Plasma_Shields_2;
const BuildType* Protoss_Plasma_Shields_3;
const BuildType* U_238_Shells;
const BuildType* Ion_Thrusters;
const BuildType* Titan_Reactor;
const BuildType* Ocular_Implants;
const BuildType* Moebius_Reactor;
const BuildType* Apollo_Reactor;
const BuildType* Colossus_Reactor;
const BuildType* Ventral_Sacs;
const BuildType* Antennae;
const BuildType* Pneumatized_Carapace;
const BuildType* Metabolic_Boost;
const BuildType* Adrenal_Glands;
const BuildType* Muscular_Augments;
const BuildType* Grooved_Spines;
const BuildType* Gamete_Meiosis;
const BuildType* Metasynaptic_Node;
const BuildType* Singularity_Charge;
const BuildType* Leg_Enhancements;
const BuildType* Scarab_Damage;
const BuildType* Reaver_Capacity;
const BuildType* Gravitic_Drive;
const BuildType* Sensor_Array;
const BuildType* Gravitic_Boosters;
const BuildType* Khaydarin_Amulet;
const BuildType* Apial_Sensors;
const BuildType* Gravitic_Thrusters;
const BuildType* Carrier_Capacity;
const BuildType* Khaydarin_Core;
const BuildType* Argus_Jewel;
const BuildType* Argus_Talisman;
const BuildType* Caduceus_Reactor;
const BuildType* Chitinous_Plating;
const BuildType* Anabolic_Synthesis;
const BuildType* Charon_Boosters;
const BuildType* Stim_Packs;
const BuildType* Lockdown;
const BuildType* EMP_Shockwave;
const BuildType* Spider_Mines;
const BuildType* Scanner_Sweep;
const BuildType* Tank_Siege_Mode;
const BuildType* Defensive_Matrix;
const BuildType* Irradiate;
const BuildType* Yamato_Gun;
const BuildType* Cloaking_Field;
const BuildType* Personnel_Cloaking;
const BuildType* Burrowing;
const BuildType* Infestation;
const BuildType* Spawn_Broodlings;
const BuildType* Dark_Swarm;
const BuildType* Plague;
const BuildType* Consume;
const BuildType* Ensnare;
const BuildType* Parasite;
const BuildType* Psionic_Storm;
const BuildType* Hallucination;
const BuildType* Recall;
const BuildType* Stasis_Field;
const BuildType* Archon_Warp;
const BuildType* Restoration;
const BuildType* Disruption_Web;
const BuildType* Mind_Control;
const BuildType* Dark_Archon_Meld;
const BuildType* Feedback;
const BuildType* Optical_Flare;
const BuildType* Maelstrom;
const BuildType* Lurker_Aspect;
const BuildType* Healing;

void initialize() {
  Terran_Marine = getUnitBuildType(0);
  Terran_Ghost = getUnitBuildType(1);
  Terran_Vulture = getUnitBuildType(2);
  Terran_Goliath = getUnitBuildType(3);
  Terran_Siege_Tank_Tank_Mode = getUnitBuildType(5);
  Terran_SCV = getUnitBuildType(7);
  Terran_Wraith = getUnitBuildType(8);
  Terran_Science_Vessel = getUnitBuildType(9);
  Terran_Dropship = getUnitBuildType(11);
  Terran_Battlecruiser = getUnitBuildType(12);
  Terran_Vulture_Spider_Mine = getUnitBuildType(13);
  Terran_Nuclear_Missile = getUnitBuildType(14);
  Terran_Civilian = getUnitBuildType(15);
  Terran_Siege_Tank_Siege_Mode = getUnitBuildType(30);
  Terran_Firebat = getUnitBuildType(32);
  Spell_Scanner_Sweep = getUnitBuildType(33);
  Terran_Medic = getUnitBuildType(34);
  Zerg_Larva = getUnitBuildType(35);
  Zerg_Egg = getUnitBuildType(36);
  Zerg_Zergling = getUnitBuildType(37);
  Zerg_Hydralisk = getUnitBuildType(38);
  Zerg_Ultralisk = getUnitBuildType(39);
  Zerg_Broodling = getUnitBuildType(40);
  Zerg_Drone = getUnitBuildType(41);
  Zerg_Overlord = getUnitBuildType(42);
  Zerg_Mutalisk = getUnitBuildType(43);
  Zerg_Guardian = getUnitBuildType(44);
  Zerg_Queen = getUnitBuildType(45);
  Zerg_Defiler = getUnitBuildType(46);
  Zerg_Scourge = getUnitBuildType(47);
  Zerg_Infested_Terran = getUnitBuildType(50);
  Terran_Valkyrie = getUnitBuildType(58);
  Zerg_Cocoon = getUnitBuildType(59);
  Protoss_Corsair = getUnitBuildType(60);
  Protoss_Dark_Templar = getUnitBuildType(61);
  Zerg_Devourer = getUnitBuildType(62);
  Protoss_Dark_Archon = getUnitBuildType(63);
  Protoss_Probe = getUnitBuildType(64);
  Protoss_Zealot = getUnitBuildType(65);
  Protoss_Dragoon = getUnitBuildType(66);
  Protoss_High_Templar = getUnitBuildType(67);
  Protoss_Archon = getUnitBuildType(68);
  Protoss_Shuttle = getUnitBuildType(69);
  Protoss_Scout = getUnitBuildType(70);
  Protoss_Arbiter = getUnitBuildType(71);
  Protoss_Carrier = getUnitBuildType(72);
  Protoss_Interceptor = getUnitBuildType(73);
  Protoss_Reaver = getUnitBuildType(83);
  Protoss_Observer = getUnitBuildType(84);
  Protoss_Scarab = getUnitBuildType(85);
  Critter_Rhynadon = getUnitBuildType(89);
  Critter_Bengalaas = getUnitBuildType(90);
  Critter_Scantid = getUnitBuildType(93);
  Critter_Kakaru = getUnitBuildType(94);
  Critter_Ragnasaur = getUnitBuildType(95);
  Critter_Ursadon = getUnitBuildType(96);
  Zerg_Lurker_Egg = getUnitBuildType(97);
  Zerg_Lurker = getUnitBuildType(103);
  Spell_Disruption_Web = getUnitBuildType(105);
  Terran_Command_Center = getUnitBuildType(106);
  Terran_Comsat_Station = getUnitBuildType(107);
  Terran_Nuclear_Silo = getUnitBuildType(108);
  Terran_Supply_Depot = getUnitBuildType(109);
  Terran_Refinery = getUnitBuildType(110);
  Terran_Barracks = getUnitBuildType(111);
  Terran_Academy = getUnitBuildType(112);
  Terran_Factory = getUnitBuildType(113);
  Terran_Starport = getUnitBuildType(114);
  Terran_Control_Tower = getUnitBuildType(115);
  Terran_Science_Facility = getUnitBuildType(116);
  Terran_Covert_Ops = getUnitBuildType(117);
  Terran_Physics_Lab = getUnitBuildType(118);
  Terran_Machine_Shop = getUnitBuildType(120);
  Terran_Engineering_Bay = getUnitBuildType(122);
  Terran_Armory = getUnitBuildType(123);
  Terran_Missile_Turret = getUnitBuildType(124);
  Terran_Bunker = getUnitBuildType(125);
  Zerg_Infested_Command_Center = getUnitBuildType(130);
  Zerg_Hatchery = getUnitBuildType(131);
  Zerg_Lair = getUnitBuildType(132);
  Zerg_Hive = getUnitBuildType(133);
  Zerg_Nydus_Canal = getUnitBuildType(134);
  Zerg_Hydralisk_Den = getUnitBuildType(135);
  Zerg_Defiler_Mound = getUnitBuildType(136);
  Zerg_Greater_Spire = getUnitBuildType(137);
  Zerg_Queens_Nest = getUnitBuildType(138);
  Zerg_Evolution_Chamber = getUnitBuildType(139);
  Zerg_Ultralisk_Cavern = getUnitBuildType(140);
  Zerg_Spire = getUnitBuildType(141);
  Zerg_Spawning_Pool = getUnitBuildType(142);
  Zerg_Creep_Colony = getUnitBuildType(143);
  Zerg_Spore_Colony = getUnitBuildType(144);
  Zerg_Sunken_Colony = getUnitBuildType(146);
  Zerg_Extractor = getUnitBuildType(149);
  Protoss_Nexus = getUnitBuildType(154);
  Protoss_Robotics_Facility = getUnitBuildType(155);
  Protoss_Pylon = getUnitBuildType(156);
  Protoss_Assimilator = getUnitBuildType(157);
  Protoss_Observatory = getUnitBuildType(159);
  Protoss_Gateway = getUnitBuildType(160);
  Protoss_Photon_Cannon = getUnitBuildType(162);
  Protoss_Citadel_of_Adun = getUnitBuildType(163);
  Protoss_Cybernetics_Core = getUnitBuildType(164);
  Protoss_Templar_Archives = getUnitBuildType(165);
  Protoss_Forge = getUnitBuildType(166);
  Protoss_Stargate = getUnitBuildType(167);
  Protoss_Fleet_Beacon = getUnitBuildType(169);
  Protoss_Arbiter_Tribunal = getUnitBuildType(170);
  Protoss_Robotics_Support_Bay = getUnitBuildType(171);
  Protoss_Shield_Battery = getUnitBuildType(172);
  Resource_Mineral_Field = getUnitBuildType(176);
  Resource_Mineral_Field_Type_2 = getUnitBuildType(177);
  Resource_Mineral_Field_Type_3 = getUnitBuildType(178);
  Resource_Vespene_Geyser = getUnitBuildType(188);
  Spell_Dark_Swarm = getUnitBuildType(202);
  Special_Pit_Door = getUnitBuildType(207);
  Special_Right_Pit_Door = getUnitBuildType(208);

  Terran_Infantry_Armor_1 = getUpgradeBuildType(0, 1);
  Terran_Infantry_Armor_2 = getUpgradeBuildType(0, 2);
  Terran_Infantry_Armor_3 = getUpgradeBuildType(0, 3);
  Terran_Vehicle_Plating_1 = getUpgradeBuildType(1, 1);
  Terran_Vehicle_Plating_2 = getUpgradeBuildType(1, 2);
  Terran_Vehicle_Plating_3 = getUpgradeBuildType(1, 3);
  Terran_Ship_Plating_1 = getUpgradeBuildType(2, 1);
  Terran_Ship_Plating_2 = getUpgradeBuildType(2, 2);
  Terran_Ship_Plating_3 = getUpgradeBuildType(2, 3);
  Zerg_Carapace_1 = getUpgradeBuildType(3, 1);
  Zerg_Carapace_2 = getUpgradeBuildType(3, 2);
  Zerg_Carapace_3 = getUpgradeBuildType(3, 3);
  Zerg_Flyer_Carapace_1 = getUpgradeBuildType(4, 1);
  Zerg_Flyer_Carapace_2 = getUpgradeBuildType(4, 2);
  Zerg_Flyer_Carapace_3 = getUpgradeBuildType(4, 3);
  Protoss_Ground_Armor_1 = getUpgradeBuildType(5, 1);
  Protoss_Ground_Armor_2 = getUpgradeBuildType(5, 2);
  Protoss_Ground_Armor_3 = getUpgradeBuildType(5, 3);
  Protoss_Air_Armor_1 = getUpgradeBuildType(6, 1);
  Protoss_Air_Armor_2 = getUpgradeBuildType(6, 2);
  Protoss_Air_Armor_3 = getUpgradeBuildType(6, 3);
  Terran_Infantry_Weapons_1 = getUpgradeBuildType(7, 1);
  Terran_Infantry_Weapons_2 = getUpgradeBuildType(7, 2);
  Terran_Infantry_Weapons_3 = getUpgradeBuildType(7, 3);
  Terran_Vehicle_Weapons_1 = getUpgradeBuildType(8, 1);
  Terran_Vehicle_Weapons_2 = getUpgradeBuildType(8, 2);
  Terran_Vehicle_Weapons_3 = getUpgradeBuildType(8, 3);
  Terran_Ship_Weapons_1 = getUpgradeBuildType(9, 1);
  Terran_Ship_Weapons_2 = getUpgradeBuildType(9, 2);
  Terran_Ship_Weapons_3 = getUpgradeBuildType(9, 3);
  Zerg_Melee_Attacks_1 = getUpgradeBuildType(10, 1);
  Zerg_Melee_Attacks_2 = getUpgradeBuildType(10, 2);
  Zerg_Melee_Attacks_3 = getUpgradeBuildType(10, 3);
  Zerg_Missile_Attacks_1 = getUpgradeBuildType(11, 1);
  Zerg_Missile_Attacks_2 = getUpgradeBuildType(11, 2);
  Zerg_Missile_Attacks_3 = getUpgradeBuildType(11, 3);
  Zerg_Flyer_Attacks_1 = getUpgradeBuildType(12, 1);
  Zerg_Flyer_Attacks_2 = getUpgradeBuildType(12, 2);
  Zerg_Flyer_Attacks_3 = getUpgradeBuildType(12, 3);
  Protoss_Ground_Weapons_1 = getUpgradeBuildType(13, 1);
  Protoss_Ground_Weapons_2 = getUpgradeBuildType(13, 2);
  Protoss_Ground_Weapons_3 = getUpgradeBuildType(13, 3);
  Protoss_Air_Weapons_1 = getUpgradeBuildType(14, 1);
  Protoss_Air_Weapons_2 = getUpgradeBuildType(14, 2);
  Protoss_Air_Weapons_3 = getUpgradeBuildType(14, 3);
  Protoss_Plasma_Shields_1 = getUpgradeBuildType(15, 1);
  Protoss_Plasma_Shields_2 = getUpgradeBuildType(15, 2);
  Protoss_Plasma_Shields_3 = getUpgradeBuildType(15, 3);

  U_238_Shells = getUpgradeBuildType(BWAPI::UpgradeTypes::U_238_Shells);
  Ion_Thrusters = getUpgradeBuildType(BWAPI::UpgradeTypes::Ion_Thrusters);
  Titan_Reactor = getUpgradeBuildType(BWAPI::UpgradeTypes::Titan_Reactor);
  Ocular_Implants = getUpgradeBuildType(BWAPI::UpgradeTypes::Ocular_Implants);
  Moebius_Reactor = getUpgradeBuildType(BWAPI::UpgradeTypes::Moebius_Reactor);
  Apollo_Reactor = getUpgradeBuildType(BWAPI::UpgradeTypes::Apollo_Reactor);
  Colossus_Reactor = getUpgradeBuildType(BWAPI::UpgradeTypes::Colossus_Reactor);
  Ventral_Sacs = getUpgradeBuildType(BWAPI::UpgradeTypes::Ventral_Sacs);
  Antennae = getUpgradeBuildType(BWAPI::UpgradeTypes::Antennae);
  Pneumatized_Carapace =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Pneumatized_Carapace);
  Metabolic_Boost = getUpgradeBuildType(BWAPI::UpgradeTypes::Metabolic_Boost);
  Adrenal_Glands = getUpgradeBuildType(BWAPI::UpgradeTypes::Adrenal_Glands);
  Muscular_Augments =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Muscular_Augments);
  Grooved_Spines = getUpgradeBuildType(BWAPI::UpgradeTypes::Grooved_Spines);
  Gamete_Meiosis = getUpgradeBuildType(BWAPI::UpgradeTypes::Gamete_Meiosis);
  Metasynaptic_Node =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Metasynaptic_Node);
  Singularity_Charge =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Singularity_Charge);
  Leg_Enhancements = getUpgradeBuildType(BWAPI::UpgradeTypes::Leg_Enhancements);
  Scarab_Damage = getUpgradeBuildType(BWAPI::UpgradeTypes::Scarab_Damage);
  Reaver_Capacity = getUpgradeBuildType(BWAPI::UpgradeTypes::Reaver_Capacity);
  Gravitic_Drive = getUpgradeBuildType(BWAPI::UpgradeTypes::Gravitic_Drive);
  Sensor_Array = getUpgradeBuildType(BWAPI::UpgradeTypes::Sensor_Array);
  Gravitic_Boosters =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Gravitic_Boosters);
  Khaydarin_Amulet = getUpgradeBuildType(BWAPI::UpgradeTypes::Khaydarin_Amulet);
  Apial_Sensors = getUpgradeBuildType(BWAPI::UpgradeTypes::Apial_Sensors);
  Gravitic_Thrusters =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Gravitic_Thrusters);
  Carrier_Capacity = getUpgradeBuildType(BWAPI::UpgradeTypes::Carrier_Capacity);
  Khaydarin_Core = getUpgradeBuildType(BWAPI::UpgradeTypes::Khaydarin_Core);
  Argus_Jewel = getUpgradeBuildType(BWAPI::UpgradeTypes::Argus_Jewel);
  Argus_Talisman = getUpgradeBuildType(BWAPI::UpgradeTypes::Argus_Talisman);
  Caduceus_Reactor = getUpgradeBuildType(BWAPI::UpgradeTypes::Caduceus_Reactor);
  Chitinous_Plating =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Chitinous_Plating);
  Anabolic_Synthesis =
      getUpgradeBuildType(BWAPI::UpgradeTypes::Anabolic_Synthesis);
  Charon_Boosters = getUpgradeBuildType(BWAPI::UpgradeTypes::Charon_Boosters);

  Stim_Packs = getTechBuildType(BWAPI::TechTypes::Stim_Packs);
  Lockdown = getTechBuildType(BWAPI::TechTypes::Lockdown);
  EMP_Shockwave = getTechBuildType(BWAPI::TechTypes::EMP_Shockwave);
  Spider_Mines = getTechBuildType(BWAPI::TechTypes::Spider_Mines);
  Scanner_Sweep = getTechBuildType(BWAPI::TechTypes::Scanner_Sweep);
  Tank_Siege_Mode = getTechBuildType(BWAPI::TechTypes::Tank_Siege_Mode);
  Defensive_Matrix = getTechBuildType(BWAPI::TechTypes::Defensive_Matrix);
  Irradiate = getTechBuildType(BWAPI::TechTypes::Irradiate);
  Yamato_Gun = getTechBuildType(BWAPI::TechTypes::Yamato_Gun);
  Cloaking_Field = getTechBuildType(BWAPI::TechTypes::Cloaking_Field);
  Personnel_Cloaking = getTechBuildType(BWAPI::TechTypes::Personnel_Cloaking);
  Burrowing = getTechBuildType(BWAPI::TechTypes::Burrowing);
  Infestation = getTechBuildType(BWAPI::TechTypes::Infestation);
  Spawn_Broodlings = getTechBuildType(BWAPI::TechTypes::Spawn_Broodlings);
  Dark_Swarm = getTechBuildType(BWAPI::TechTypes::Dark_Swarm);
  Plague = getTechBuildType(BWAPI::TechTypes::Plague);
  Consume = getTechBuildType(BWAPI::TechTypes::Consume);
  Ensnare = getTechBuildType(BWAPI::TechTypes::Ensnare);
  Parasite = getTechBuildType(BWAPI::TechTypes::Parasite);
  Psionic_Storm = getTechBuildType(BWAPI::TechTypes::Psionic_Storm);
  Hallucination = getTechBuildType(BWAPI::TechTypes::Hallucination);
  Recall = getTechBuildType(BWAPI::TechTypes::Recall);
  Stasis_Field = getTechBuildType(BWAPI::TechTypes::Stasis_Field);
  Archon_Warp = getTechBuildType(BWAPI::TechTypes::Archon_Warp);
  Restoration = getTechBuildType(BWAPI::TechTypes::Restoration);
  Disruption_Web = getTechBuildType(BWAPI::TechTypes::Disruption_Web);
  Mind_Control = getTechBuildType(BWAPI::TechTypes::Mind_Control);
  Dark_Archon_Meld = getTechBuildType(BWAPI::TechTypes::Dark_Archon_Meld);
  Feedback = getTechBuildType(BWAPI::TechTypes::Feedback);
  Optical_Flare = getTechBuildType(BWAPI::TechTypes::Optical_Flare);
  Maelstrom = getTechBuildType(BWAPI::TechTypes::Maelstrom);
  Lurker_Aspect = getTechBuildType(BWAPI::TechTypes::Lurker_Aspect);
  Healing = getTechBuildType(BWAPI::TechTypes::Healing);

  // Populate subjectiveValue for morphed types. Order matters here!
  const_cast<BuildType*>(Zerg_Hatchery)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Extractor)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Spawning_Pool)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Hydralisk_Den)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Evolution_Chamber)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Spire)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Defiler_Mound)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Ultralisk_Cavern)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Greater_Spire)->subjectiveValue +=
      Zerg_Spire->subjectiveValue;
  const_cast<BuildType*>(Zerg_Lair)->subjectiveValue +=
      Zerg_Hatchery->subjectiveValue;
  const_cast<BuildType*>(Zerg_Hive)->subjectiveValue +=
      Zerg_Lair->subjectiveValue;
  const_cast<BuildType*>(Zerg_Creep_Colony)->subjectiveValue +=
      Zerg_Drone->subjectiveValue;
  const_cast<BuildType*>(Zerg_Spore_Colony)->subjectiveValue +=
      Zerg_Creep_Colony->subjectiveValue;
  const_cast<BuildType*>(Zerg_Sunken_Colony)->subjectiveValue +=
      Zerg_Creep_Colony->subjectiveValue;
  const_cast<BuildType*>(Zerg_Lurker)->subjectiveValue +=
      Zerg_Hydralisk->subjectiveValue;
  const_cast<BuildType*>(Zerg_Guardian)->subjectiveValue +=
      Zerg_Mutalisk->subjectiveValue;
  const_cast<BuildType*>(Zerg_Devourer)->subjectiveValue +=
      Zerg_Mutalisk->subjectiveValue;
}
} // namespace buildtypes
} // namespace torchcraft
