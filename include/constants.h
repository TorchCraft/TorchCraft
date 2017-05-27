/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#define BETTER_ENUMS_MACRO_FILE "enum_macros.h"
#include "enum.h"

namespace torchcraft {
namespace BW {

BETTER_ENUM(
    Command,
    int,
    // no arguments
    Quit = 0, // leave the game
    Restart =
        1, // restart the game. Much faster, but doesn't work in multiplayer

    MapHack = 2, // remove fog of war
    RequestImage = 3,
    ExitProcess = 4,
    Noop = 5, // do nothing

    // one argument
    SetSpeed = 6, // sets the game speed (int)
    SetLog = 7, // activates logging (bool)
    SetGui = 8, // activates drawing and text in SC (bool)
    SetFrameskip = 9, // number of frames to skip (int)
    SetCmdOptim = 10, // reduce bot APM (0-6)
    SetCombineFrames = 11, // combine n frames before sending (int)

    // Sets the map with BWAPI->setMap and by writing to the config. Is not
    // thread-safe. However, as long as the next connect finishes after set_map,
    // you are guaranteed the map will be what you want.
    SetMap = 12,
    SetMulti = 13,

    // arguments: (unit ID, command, target id, target x, target y, extra)
    // (x, y) are walktiles instead of pixels
    // otherwise this corresponds exactly to BWAPI::UnitCommand
    CommandUnit = 14,
    CommandUnitProtected = 15,

    // arguments: (command, args)
    // For documentation about args, see usercommandtypes
    CommandUser = 16,
		CommandOpenBW = 17,

    // BWAPI drawing routines
    DrawLine = 18, //  x1, y1, x2, y2, color index
    DrawUnitLine = 19, // uid1, uid2, color index
    DrawUnitPosLine = 20, // uid, x2, y2, color index
    DrawCircle = 21, //  x, y, radius, color index
    DrawUnitCircle = 22, // uid, radius, color index
    DrawText = 23, // x, y plus text arg
    DrawTextScreen = 24, // x, y plus text arg

    MAX)

BETTER_ENUM(
    OpenBWCommandType,
    int,
    // two args
    KillUnit = 0, // uid
    // four args
    SpawnUnit = 1, // playerid, type, x, y

    OPENBW_COMMAND_END = 2)

BETTER_ENUM(
    UserCommandType,
    int,
    // one arg
    Move_Screen_Up = 0, // arguments: magnitude (amount of pixels)
    Move_Screen_Down = 1, // arguments: magnitude (amount of pixels)
    Move_Screen_Left = 2, // arguments: magnitude (amount of pixels)
    Move_Screen_Right = 3, // arguments: magnitude (amount of pixels)

    // two args
    Move_Screen_To_Pos = 4, // arguments: (x, y)
    Right_Click = 5, // arguments: (x, y)

    USER_COMMAND_END = 6)

BETTER_ENUM(
    UnitCommandType,
    int,
    // corresponds to BWAPI::UnitCommandTypes::Enum
    Attack_Move = 0,
    Attack_Unit = 1,
    Build = 2,
    Build_Addon = 3,
    Train = 4,
    Morph = 5,
    Research = 6,
    Upgrade = 7,
    Set_Rally_Position = 8,
    Set_Rally_Unit = 9,
    Move = 10,
    Patrol = 11,
    Hold_Position = 12,
    Stop = 13,
    Follow = 14,
    Gather = 15,
    Return_Cargo = 16,
    Repair = 17,
    Burrow = 18,
    Unburrow = 19,
    Cloak = 20,
    Decloak = 21,
    Siege = 22,
    Unsiege = 23,
    Lift = 24,
    Land = 25,
    Load = 26,
    Unload = 27,
    Unload_All = 28,
    Unload_All_Position = 29,
    Right_Click_Position = 30,
    Right_Click_Unit = 31,
    Halt_Construction = 32,
    Cancel_Construction = 33,
    Cancel_Addon = 34,
    Cancel_Train = 35,
    Cancel_Train_Slot = 36,
    Cancel_Morph = 37,
    Cancel_Research = 38,
    Cancel_Upgrade = 39,
    Use_Tech = 40,
    Use_Tech_Position = 41,
    Use_Tech_Unit = 42,
    Place_COP = 43,
    None = 44,
    Unknown = 45,
    MAX = 46)

BETTER_ENUM(
    Order,
    int,
    // corresponds to BWAPI::Orders::Enum
    Die = 0,
    Stop = 1,
    Guard = 2,
    PlayerGuard = 3,
    TurretGuard = 4,
    BunkerGuard = 5,
    Move = 6,
    ReaverStop = 7,
    Attack1 = 8,
    Attack2 = 9,
    AttackUnit = 10,
    AttackFixedRange = 11,
    AttackTile = 12,
    Hover = 13,
    AttackMove = 14,
    InfestedCommandCenter = 15,
    UnusedNothing = 16,
    UnusedPowerup = 17,
    TowerGuard = 18,
    TowerAttack = 19,
    VultureMine = 20,
    StayInRange = 21,
    TurretAttack = 22,
    Nothing = 23,
    Unused_24 = 24,
    DroneStartBuild = 25,
    DroneBuild = 26,
    CastInfestation = 27,
    MoveToInfest = 28,
    InfestingCommandCenter = 29,
    PlaceBuilding = 30,
    PlaceProtossBuilding = 31,
    CreateProtossBuilding = 32,
    ConstructingBuilding = 33,
    Repair = 34,
    MoveToRepair = 35,
    PlaceAddon = 36,
    BuildAddon = 37,
    Train = 38,
    RallyPointUnit = 39,
    RallyPointTile = 40,
    ZergBirth = 41,
    ZergUnitMorph = 42,
    ZergBuildingMorph = 43,
    IncompleteBuilding = 44,
    IncompleteMorphing = 45,
    BuildNydusExit = 46,
    EnterNydusCanal = 47,
    IncompleteWarping = 48,
    Follow = 49,
    Carrier = 50,
    ReaverCarrierMove = 51,
    CarrierStop = 52,
    CarrierAttack = 53,
    CarrierMoveToAttack = 54,
    CarrierIgnore2 = 55,
    CarrierFight = 56,
    CarrierHoldPosition = 57,
    Reaver = 58,
    ReaverAttack = 59,
    ReaverMoveToAttack = 60,
    ReaverFight = 61,
    ReaverHoldPosition = 62,
    TrainFighter = 63,
    InterceptorAttack = 64,
    ScarabAttack = 65,
    RechargeShieldsUnit = 66,
    RechargeShieldsBattery = 67,
    ShieldBattery = 68,
    InterceptorReturn = 69,
    DroneLand = 70,
    BuildingLand = 71,
    BuildingLiftOff = 72,
    DroneLiftOff = 73,
    LiftingOff = 74,
    ResearchTech = 75,
    Upgrade = 76,
    Larva = 77,
    SpawningLarva = 78,
    Harvest1 = 79,
    Harvest2 = 80,
    MoveToGas = 81,
    WaitForGas = 82,
    HarvestGas = 83,
    ReturnGas = 84,
    MoveToMinerals = 85,
    WaitForMinerals = 86,
    MiningMinerals = 87,
    Harvest3 = 88,
    Harvest4 = 89,
    ReturnMinerals = 90,
    Interrupted = 91,
    EnterTransport = 92,
    PickupIdle = 93,
    PickupTransport = 94,
    PickupBunker = 95,
    Pickup4 = 96,
    PowerupIdle = 97,
    Sieging = 98,
    Unsieging = 99,
    WatchTarget = 100,
    InitCreepGrowth = 101,
    SpreadCreep = 102,
    StoppingCreepGrowth = 103,
    GuardianAspect = 104,
    ArchonWarp = 105,
    CompletingArchonSummon = 106,
    HoldPosition = 107,
    QueenHoldPosition = 108,
    Cloak = 109,
    Decloak = 110,
    Unload = 111,
    MoveUnload = 112,
    FireYamatoGun = 113,
    MoveToFireYamatoGun = 114,
    CastLockdown = 115,
    Burrowing = 116,
    Burrowed = 117,
    Unburrowing = 118,
    CastDarkSwarm = 119,
    CastParasite = 120,
    CastSpawnBroodlings = 121,
    CastEMPShockwave = 122,
    NukeWait = 123,
    NukeTrain = 124,
    NukeLaunch = 125,
    NukePaint = 126,
    NukeUnit = 127,
    CastNuclearStrike = 128,
    NukeTrack = 129,
    InitializeArbiter = 130,
    CloakNearbyUnits = 131,
    PlaceMine = 132,
    RightClickAction = 133,
    SuicideUnit = 134,
    SuicideLocation = 135,
    SuicideHoldPosition = 136,
    CastRecall = 137,
    Teleport = 138,
    CastScannerSweep = 139,
    Scanner = 140,
    CastDefensiveMatrix = 141,
    CastPsionicStorm = 142,
    CastIrradiate = 143,
    CastPlague = 144,
    CastConsume = 145,
    CastEnsnare = 146,
    CastStasisField = 147,
    CastHallucination = 148,
    Hallucination2 = 149,
    ResetCollision = 150,
    ResetHarvestCollision = 151,
    Patrol = 152,
    CTFCOPInit = 153,
    CTFCOPStarted = 154,
    CTFCOP2 = 155,
    ComputerAI = 156,
    AtkMoveEP = 157,
    HarassMove = 158,
    AIPatrol = 159,
    GuardPost = 160,
    RescuePassive = 161,
    Neutral = 162,
    ComputerReturn = 163,
    InitializePsiProvider = 164,
    SelfDestructing = 165,
    Critter = 166,
    HiddenGun = 167,
    OpenDoor = 168,
    CloseDoor = 169,
    HideTrap = 170,
    RevealTrap = 171,
    EnableDoodad = 172,
    DisableDoodad = 173,
    WarpIn = 174,
    Medic = 175,
    MedicHeal = 176,
    HealMove = 177,
    MedicHoldPosition = 178,
    MedicHealToIdle = 179,
    CastRestoration = 180,
    CastDisruptionWeb = 181,
    CastMindControl = 182,
    DarkArchonMeld = 183,
    CastFeedback = 184,
    CastOpticalFlare = 185,
    CastMaelstrom = 186,
    JunkYardDog = 187,
    Fatal = 188,
    None = 189,
    Unknown = 190,
    MAX = 191)

BETTER_ENUM(
    TechType,
    int,
    // corresponds to BWAPI::TechTypes::Enum
    Stim_Packs = 0,
    Lockdown = 1,
    EMP_Shockwave = 2,
    Spider_Mines = 3,
    Scanner_Sweep = 4,
    Tank_Siege_Mode = 5,
    Defensive_Matrix = 6,
    Irradiate = 7,
    Yamato_Gun = 8,
    Cloaking_Field = 9,
    Personnel_Cloaking = 10,
    Burrowing = 11,
    Infestation = 12,
    Spawn_Broodlings = 13,
    Dark_Swarm = 14,
    Plague = 15,
    Consume = 16,
    Ensnare = 17,
    Parasite = 18,
    Psionic_Storm = 19,
    Hallucination = 20,
    Recall = 21,
    Stasis_Field = 22,
    Archon_Warp = 23,
    Restoration = 24,
    Disruption_Web = 25,
    Unused_26 = 26,
    Mind_Control = 27,
    Dark_Archon_Meld = 28,
    Feedback = 29,
    Optical_Flare = 30,
    Maelstrom = 31,
    Lurker_Aspect = 32,
    Unused_33 = 33,
    Healing = 34,
    None = 44,
    Nuclear_Strike = 45,
    Unknown = 46,
    MAX = 47)

BETTER_ENUM(
    UnitType,
    int,
    // corresponds to BWAPI::UnitTypes::Enum
    Terran_Marine = 0,
    Terran_Ghost = 1,
    Terran_Vulture = 2,
    Terran_Goliath = 3,
    Terran_Siege_Tank_Tank_Mode = 5,
    Terran_SCV = 7,
    Terran_Wraith = 8,
    Terran_Science_Vessel = 9,
    Terran_Dropship = 11,
    Terran_Battlecruiser = 12,
    Terran_Vulture_Spider_Mine = 13,
    Terran_Nuclear_Missile = 14,
    Terran_Civilian = 15,
    Terran_Siege_Tank_Siege_Mode = 30,
    Terran_Firebat = 32,
    Spell_Scanner_Sweep = 33,
    Terran_Medic = 34,
    Zerg_Larva = 35,
    Zerg_Egg = 36,
    Zerg_Zergling = 37,
    Zerg_Hydralisk = 38,
    Zerg_Ultralisk = 39,
    Zerg_Broodling = 40,
    Zerg_Drone = 41,
    Zerg_Overlord = 42,
    Zerg_Mutalisk = 43,
    Zerg_Guardian = 44,
    Zerg_Queen = 45,
    Zerg_Defiler = 46,
    Zerg_Scourge = 47,
    Zerg_Infested_Terran = 50,
    Terran_Valkyrie = 58,
    Zerg_Cocoon = 59,
    Protoss_Corsair = 60,
    Protoss_Dark_Templar = 61,
    Zerg_Devourer = 62,
    Protoss_Dark_Archon = 63,
    Protoss_Probe = 64,
    Protoss_Zealot = 65,
    Protoss_Dragoon = 66,
    Protoss_High_Templar = 67,
    Protoss_Archon = 68,
    Protoss_Shuttle = 69,
    Protoss_Scout = 70,
    Protoss_Arbiter = 71,
    Protoss_Carrier = 72,
    Protoss_Interceptor = 73,
    Protoss_Reaver = 83,
    Protoss_Observer = 84,
    Protoss_Scarab = 85,
    Critter_Rhynadon = 89,
    Critter_Bengalaas = 90,
    Critter_Scantid = 93,
    Critter_Kakaru = 94,
    Critter_Ragnasaur = 95,
    Critter_Ursadon = 96,
    Zerg_Lurker_Egg = 97,
    Zerg_Lurker = 103,
    Spell_Disruption_Web = 105,
    Terran_Command_Center = 106,
    Terran_Comsat_Station = 107,
    Terran_Nuclear_Silo = 108,
    Terran_Supply_Depot = 109,
    Terran_Refinery = 110,
    Terran_Barracks = 111,
    Terran_Academy = 112,
    Terran_Factory = 113,
    Terran_Starport = 114,
    Terran_Control_Tower = 115,
    Terran_Science_Facility = 116,
    Terran_Covert_Ops = 117,
    Terran_Physics_Lab = 118,
    Terran_Machine_Shop = 120,
    Terran_Engineering_Bay = 122,
    Terran_Armory = 123,
    Terran_Missile_Turret = 124,
    Terran_Bunker = 125,
    Zerg_Infested_Command_Center = 130,
    Zerg_Hatchery = 131,
    Zerg_Lair = 132,
    Zerg_Hive = 133,
    Zerg_Nydus_Canal = 134,
    Zerg_Hydralisk_Den = 135,
    Zerg_Defiler_Mound = 136,
    Zerg_Greater_Spire = 137,
    Zerg_Queens_Nest = 138,
    Zerg_Evolution_Chamber = 139,
    Zerg_Ultralisk_Cavern = 140,
    Zerg_Spire = 141,
    Zerg_Spawning_Pool = 142,
    Zerg_Creep_Colony = 143,
    Zerg_Spore_Colony = 144,
    Zerg_Sunken_Colony = 146,
    Zerg_Extractor = 149,
    Protoss_Nexus = 154,
    Protoss_Robotics_Facility = 155,
    Protoss_Pylon = 156,
    Protoss_Assimilator = 157,
    Protoss_Observatory = 159,
    Protoss_Gateway = 160,
    Protoss_Photon_Cannon = 162,
    Protoss_Citadel_of_Adun = 163,
    Protoss_Cybernetics_Core = 164,
    Protoss_Templar_Archives = 165,
    Protoss_Forge = 166,
    Protoss_Stargate = 167,
    Protoss_Fleet_Beacon = 169,
    Protoss_Arbiter_Tribunal = 170,
    Protoss_Robotics_Support_Bay = 171,
    Protoss_Shield_Battery = 172,
    Resource_Mineral_Field = 176,
    Resource_Mineral_Field_Type_2 = 177,
    Resource_Mineral_Field_Type_3 = 178,
    Resource_Vespene_Geyser = 188,
    Spell_Dark_Swarm = 202,
    Special_Pit_Door = 207,
    Special_Right_Pit_Door = 208,
    MAX = 233)

BETTER_ENUM(
    BulletType,
    int,
    // corresponds to BWAPI::BulletTypes::Enum
    Melee = 0,
    Fusion_Cutter_Hit = 141,
    Gauss_Rifle_Hit = 142,
    C_10_Canister_Rifle_Hit = 143,
    Gemini_Missiles = 144,
    Fragmentation_Grenade = 145,
    Longbolt_Missile = 146,
    Unused_Lockdown = 147,
    ATS_ATA_Laser_Battery = 148,
    Burst_Lasers = 149,
    Arclite_Shock_Cannon_Hit = 150,
    EMP_Missile = 151,
    Dual_Photon_Blasters_Hit = 152,
    Particle_Beam_Hit = 153,
    Anti_Matter_Missile = 154,
    Pulse_Cannon = 155,
    Psionic_Shockwave_Hit = 156,
    Psionic_Storm = 157,
    Yamato_Gun = 158,
    Phase_Disruptor = 159,
    STA_STS_Cannon_Overlay = 160,
    Sunken_Colony_Tentacle = 161,
    Venom_Unused = 162,
    Acid_Spore = 163,
    Plasma_Drip_Unused = 164,
    Glave_Wurm = 165,
    Seeker_Spores = 166,
    Queen_Spell_Carrier = 167,
    Plague_Cloud = 168,
    Consume = 169,
    Ensnare = 170,
    Needle_Spine_Hit = 171,
    Invisible = 172,
    Optical_Flare_Grenade = 201,
    Halo_Rockets = 202,
    Subterranean_Spines = 203,
    Corrosive_Acid_Shot = 204,
    Corrosive_Acid_Hit = 205,
    Neutron_Flare = 206,
    None = 209,
    Unknown = 210,
    MAX = 211)

BETTER_ENUM(
    WeaponType,
    int,
    // corresponds to BWAPI::WeaponTypes::Enum
    Gauss_Rifle = 0,
    Gauss_Rifle_Jim_Raynor = 1,
    C_10_Canister_Rifle = 2,
    C_10_Canister_Rifle_Sarah_Kerrigan = 3,
    Fragmentation_Grenade = 4,
    Fragmentation_Grenade_Jim_Raynor = 5,
    Spider_Mines = 6,
    Twin_Autocannons = 7,
    Hellfire_Missile_Pack = 8,
    Twin_Autocannons_Alan_Schezar = 9,
    Hellfire_Missile_Pack_Alan_Schezar = 10,
    Arclite_Cannon = 11,
    Arclite_Cannon_Edmund_Duke = 12,
    Fusion_Cutter = 13,

    Gemini_Missiles = 15,
    Burst_Lasers = 16,
    Gemini_Missiles_Tom_Kazansky = 17,
    Burst_Lasers_Tom_Kazansky = 18,
    ATS_Laser_Battery = 19,
    ATA_Laser_Battery = 20,
    ATS_Laser_Battery_Hero = 21,
    ATA_Laser_Battery_Hero = 22,
    ATS_Laser_Battery_Hyperion = 23,
    ATA_Laser_Battery_Hyperion = 24,
    Flame_Thrower = 25,
    Flame_Thrower_Gui_Montag = 26,
    Arclite_Shock_Cannon = 27,
    Arclite_Shock_Cannon_Edmund_Duke = 28,
    Longbolt_Missile = 29,
    Yamato_Gun = 30,
    Nuclear_Strike = 31,
    Lockdown = 32,
    EMP_Shockwave = 33,
    Irradiate = 34,
    Claws = 35,
    Claws_Devouring_One = 36,
    Claws_Infested_Kerrigan = 37,
    Needle_Spines = 38,
    Needle_Spines_Hunter_Killer = 39,
    Kaiser_Blades = 40,
    Kaiser_Blades_Torrasque = 41,
    Toxic_Spores = 42,
    Spines = 43,

    Acid_Spore = 46,
    Acid_Spore_Kukulza = 47,
    Glave_Wurm = 48,
    Glave_Wurm_Kukulza = 49,

    Seeker_Spores = 52,
    Subterranean_Tentacle = 53,
    Suicide_Infested_Terran = 54,
    Suicide_Scourge = 55,
    Parasite = 56,
    Spawn_Broodlings = 57,
    Ensnare = 58,
    Dark_Swarm = 59,
    Plague = 60,
    Consume = 61,
    Particle_Beam = 62,

    Psi_Blades = 64,
    Psi_Blades_Fenix = 65,
    Phase_Disruptor = 66,
    Phase_Disruptor_Fenix = 67,

    Psi_Assault = 69,
    Psionic_Shockwave = 70,
    Psionic_Shockwave_TZ_Archon = 71,

    Dual_Photon_Blasters = 73,
    Anti_Matter_Missiles = 74,
    Dual_Photon_Blasters_Mojo = 75,
    Anti_Matter_Missiles_Mojo = 76,
    Phase_Disruptor_Cannon = 77,
    Phase_Disruptor_Cannon_Danimoth = 78,
    Pulse_Cannon = 79,
    STS_Photon_Cannon = 80,
    STA_Photon_Cannon = 81,
    Scarab = 82,
    Stasis_Field = 83,
    Psionic_Storm = 84,
    Warp_Blades_Zeratul = 85,
    Warp_Blades_Hero = 86,

    Platform_Laser_Battery = 92,
    Independant_Laser_Battery = 93,

    Twin_Autocannons_Floor_Trap = 96,
    Hellfire_Missile_Pack_Wall_Trap = 97,
    Flame_Thrower_Wall_Trap = 98,
    Hellfire_Missile_Pack_Floor_Trap = 99,

    Neutron_Flare = 100,
    Disruption_Web = 101,
    Restoration = 102,
    Halo_Rockets = 103,
    Corrosive_Acid = 104,
    Mind_Control = 105,
    Feedback = 106,
    Optical_Flare = 107,
    Maelstrom = 108,
    Subterranean_Spines = 109,

    Warp_Blades = 111,
    C_10_Canister_Rifle_Samir_Duran = 112,
    C_10_Canister_Rifle_Infested_Duran = 113,
    Dual_Photon_Blasters_Artanis = 114,
    Anti_Matter_Missiles_Artanis = 115,
    C_10_Canister_Rifle_Alexei_Stukov = 116,

    None = 130,
    Unknown,
    MAX)

BETTER_ENUM(
    UnitSize,
    int,
    // corresponds to BWAPI::UnitSizeTypes::Enum
    Independent = 0,
    Small = 1,
    Medium = 2,
    Large = 3)

BETTER_ENUM(
    DamageType,
    int,
    // corresponds to BWAPI::DamageTypes::Enum
    Independent = 0,
    Explosive = 1,
    Concussive = 2,
    Normal = 3,
    Ignore_Armor = 4,
    None = 5)

BETTER_ENUM(
    Color,
    int,
    Black = 0,
    Brown = 19,
    Grey = 74,
    Red = 111,
    Green = 117,
    Cyan = 128,
    Yellow = 135,
    Teal = 159,
    Purple = 164,
    Blue = 165,
    Orange = -179,
    White = 255)

constexpr int XYPixelsPerWalktile = 8;
constexpr int XYPixelsPerBuildtile = 32;
constexpr int XYWalktilesPerBuildtile =
    XYPixelsPerBuildtile / XYPixelsPerWalktile;
constexpr double HitProbRangedUphillDoodad = 0.53125;
constexpr double HitProbRanged = 0.99609375;

inline bool isBuilding(UnitType id) {
  return (
      id >= +UnitType::Terran_Command_Center &&
      id <= +UnitType::Protoss_Shield_Battery);
}

inline bool isWorker(UnitType id) {
  switch (id) {
    case UnitType::Protoss_Probe:
    case UnitType::Terran_SCV:
    case UnitType::Zerg_Drone:
      return true;
    default:
      break;
  }
  return false;
}

inline bool isMineralField(UnitType id) {
  switch (id) {
    case UnitType::Resource_Mineral_Field:
    case UnitType::Resource_Mineral_Field_Type_2:
    case UnitType::Resource_Mineral_Field_Type_3:
      return true;
    default:
      break;
  }
  return false;
}

inline bool isGasGeyser(UnitType id) {
  switch (id) {
    case UnitType::Resource_Vespene_Geyser:
    case UnitType::Protoss_Assimilator:
    case UnitType::Terran_Refinery:
    case UnitType::Zerg_Extractor:
      return true;
    default:
      break;
  }
  return false;
}

std::vector<UnitType> unitProductions(UnitType id);
bool unitProduces(UnitType id, UnitType product);
std::vector<Order> commandToOrders(UnitCommandType id);

// Defined in constants_static.cpp
namespace data {

// Indexed by UnitType value
extern bool CanAttack[];
extern int DimensionRight[];
extern int Height[];
extern bool IsMineralField[];
extern bool CanProduce[];
extern bool IsRefinery[];
extern bool IsResourceDepot[];
extern bool RegeneratesHP[];
extern bool IsCloakable[];
extern bool IsTwoUnitsInOneEgg[];
extern bool IsSpellcaster[];
extern int SupplyRequired[];
extern std::string AirWeapon[];
extern int BuildScore[];
extern int MaxAirHits[];
extern bool IsPowerup[];
extern bool IsBeacon[];
extern int MineralPrice[];
extern bool IsInvincible[];
extern std::string RequiredTech[];
extern int DimensionDown[];
extern bool CanBuildAddon[];
extern int DimensionLeft[];
extern bool ProducesLarva[];
extern int Armor[];
extern bool IsMechanical[];
extern bool IsBuilding[];
extern int SupplyProvided[];
extern int SightRange[];
extern int GasPrice[];
extern int MaxHitPoints[];
extern int Width[];
extern int TileWidth[];
extern bool IsHero[];
extern int SeekRange[];
extern int BuildTime[];
extern bool IsCritter[];
extern bool RequiresPsi[];
extern bool IsSpecialBuilding[];
extern std::string GroundWeapon[];
extern bool IsFlyer[];
extern std::string Size[];
extern bool IsNeutral[];
extern int MaxShields[];
extern bool HasPermanentCloak[];
extern double TopSpeed[];
extern int TileHeight[];
extern bool IsRobotic[];
extern int DimensionUp[];
extern int DestroyScore[];
extern int SpaceProvided[];
extern std::string TileSize[];
extern int HaltDistance[];
extern bool IsAddon[];
extern bool CanMove[];
extern bool IsFlyingBuilding[];
extern int MaxEnergy[];
extern bool IsDetector[];
extern bool IsOrganic[];
extern int SpaceRequired[];
extern bool IsFlagBeacon[];
extern bool IsWorker[];
extern bool IsBurrowable[];
extern std::string CloakingTech[];
extern bool IsResourceContainer[];
extern int Acceleration[];
extern bool IsSpell[];
extern bool RequiresCreep[];
extern std::string ArmorUpgrade[];
extern int MaxGroundHits[];
extern int TurnRadius[];
extern std::string GetRace[];

extern std::unordered_map<UnitType, int> TotalMineralPrice;
extern std::unordered_map<UnitType, int> TotalGasPrice;
void init();

} // namespace data

inline bool isResourceDepot(UnitType id) {
  return data::IsResourceDepot[id];
}


} // namespace BW
} // namespace torchcraft

// Specialized hash functions for enums declared above
namespace std {
template <>
struct hash<torchcraft::BW::UnitType> {
  typedef torchcraft::BW::UnitType argument_type;
  typedef std::size_t result_type;
  result_type operator()(argument_type const& s) const {
    return std::hash<torchcraft::BW::UnitType::_integral>{}(s._to_integral());
  }
};
} // namespace std
