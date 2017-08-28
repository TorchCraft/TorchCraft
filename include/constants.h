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

// With MSVC, we are limited by the maximum number of macro arguments that is
// pretty restrictive (127). To get around this, we stick to better-enum's
// default limits and included the fully expanded enum definitions, generated
// with GCC, for enums that are too large.
#ifdef _MSC_VER
#define TC_EXPAND_LARGE_ENUMS
#else // _MSC_VER
#define BETTER_ENUMS_MACRO_FILE "enum_macros.h"
#endif // _MSC_VER

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
    CommandOpenbw = 17,

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

#ifndef TC_EXPAND_LARGE_ENUMS
// Be sure to update the expanded version below when changing this
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
#else // TC_EXPAND_LARGE_ENUMS
// Macro above expanded with GCC
namespace better_enums_data_Order {} class Order { private: typedef ::better_enums::optional<Order> _optional; typedef ::better_enums::optional<std::size_t> _optional_index; public: typedef int _integral; enum _enumerated : int { Die = 0, Stop = 1, Guard = 2, PlayerGuard = 3, TurretGuard = 4, BunkerGuard = 5, Move = 6, ReaverStop = 7, Attack1 = 8, Attack2 = 9, AttackUnit = 10, AttackFixedRange = 11, AttackTile = 12, Hover = 13, AttackMove = 14, InfestedCommandCenter = 15, UnusedNothing = 16, UnusedPowerup = 17, TowerGuard = 18, TowerAttack = 19, VultureMine = 20, StayInRange = 21, TurretAttack = 22, Nothing = 23, Unused_24 = 24, DroneStartBuild = 25, DroneBuild = 26, CastInfestation = 27, MoveToInfest = 28, InfestingCommandCenter = 29, PlaceBuilding = 30, PlaceProtossBuilding = 31, CreateProtossBuilding = 32, ConstructingBuilding = 33, Repair = 34, MoveToRepair = 35, PlaceAddon = 36, BuildAddon = 37, Train = 38, RallyPointUnit = 39, RallyPointTile = 40, ZergBirth = 41, ZergUnitMorph = 42, ZergBuildingMorph = 43, IncompleteBuilding = 44, IncompleteMorphing = 45, BuildNydusExit = 46, EnterNydusCanal = 47, IncompleteWarping = 48, Follow = 49, Carrier = 50, ReaverCarrierMove = 51, CarrierStop = 52, CarrierAttack = 53, CarrierMoveToAttack = 54, CarrierIgnore2 = 55, CarrierFight = 56, CarrierHoldPosition = 57, Reaver = 58, ReaverAttack = 59, ReaverMoveToAttack = 60, ReaverFight = 61, ReaverHoldPosition = 62, TrainFighter = 63, InterceptorAttack = 64, ScarabAttack = 65, RechargeShieldsUnit = 66, RechargeShieldsBattery = 67, ShieldBattery = 68, InterceptorReturn = 69, DroneLand = 70, BuildingLand = 71, BuildingLiftOff = 72, DroneLiftOff = 73, LiftingOff = 74, ResearchTech = 75, Upgrade = 76, Larva = 77, SpawningLarva = 78, Harvest1 = 79, Harvest2 = 80, MoveToGas = 81, WaitForGas = 82, HarvestGas = 83, ReturnGas = 84, MoveToMinerals = 85, WaitForMinerals = 86, MiningMinerals = 87, Harvest3 = 88, Harvest4 = 89, ReturnMinerals = 90, Interrupted = 91, EnterTransport = 92, PickupIdle = 93, PickupTransport = 94, PickupBunker = 95, Pickup4 = 96, PowerupIdle = 97, Sieging = 98, Unsieging = 99, WatchTarget = 100, InitCreepGrowth = 101, SpreadCreep = 102, StoppingCreepGrowth = 103, GuardianAspect = 104, ArchonWarp = 105, CompletingArchonSummon = 106, HoldPosition = 107, QueenHoldPosition = 108, Cloak = 109, Decloak = 110, Unload = 111, MoveUnload = 112, FireYamatoGun = 113, MoveToFireYamatoGun = 114, CastLockdown = 115, Burrowing = 116, Burrowed = 117, Unburrowing = 118, CastDarkSwarm = 119, CastParasite = 120, CastSpawnBroodlings = 121, CastEMPShockwave = 122, NukeWait = 123, NukeTrain = 124, NukeLaunch = 125, NukePaint = 126, NukeUnit = 127, CastNuclearStrike = 128, NukeTrack = 129, InitializeArbiter = 130, CloakNearbyUnits = 131, PlaceMine = 132, RightClickAction = 133, SuicideUnit = 134, SuicideLocation = 135, SuicideHoldPosition = 136, CastRecall = 137, Teleport = 138, CastScannerSweep = 139, Scanner = 140, CastDefensiveMatrix = 141, CastPsionicStorm = 142, CastIrradiate = 143, CastPlague = 144, CastConsume = 145, CastEnsnare = 146, CastStasisField = 147, CastHallucination = 148, Hallucination2 = 149, ResetCollision = 150, ResetHarvestCollision = 151, Patrol = 152, CTFCOPInit = 153, CTFCOPStarted = 154, CTFCOP2 = 155, ComputerAI = 156, AtkMoveEP = 157, HarassMove = 158, AIPatrol = 159, GuardPost = 160, RescuePassive = 161, Neutral = 162, ComputerReturn = 163, InitializePsiProvider = 164, SelfDestructing = 165, Critter = 166, HiddenGun = 167, OpenDoor = 168, CloseDoor = 169, HideTrap = 170, RevealTrap = 171, EnableDoodad = 172, DisableDoodad = 173, WarpIn = 174, Medic = 175, MedicHeal = 176, HealMove = 177, MedicHoldPosition = 178, MedicHealToIdle = 179, CastRestoration = 180, CastDisruptionWeb = 181, CastMindControl = 182, DarkArchonMeld = 183, CastFeedback = 184, CastOpticalFlare = 185, CastMaelstrom = 186, JunkYardDog = 187, Fatal = 188, None = 189, Unknown = 190, MAX = 191 }; constexpr Order(_enumerated value) : _value(value) { } constexpr operator _enumerated() const { return _enumerated(_value); } constexpr _integral _to_integral() const; constexpr static Order _from_integral(_integral value); constexpr static Order _from_integral_unchecked(_integral value); constexpr static _optional _from_integral_nothrow(_integral value); const char* _to_string() const; constexpr static Order _from_string(const char *name); constexpr static _optional _from_string_nothrow(const char *name); constexpr static Order _from_string_nocase(const char *name); constexpr static _optional _from_string_nocase_nothrow(const char *name); constexpr static bool _is_valid(_integral value); constexpr static bool _is_valid(const char *name); constexpr static bool _is_valid_nocase(const char *name); typedef ::better_enums::_Iterable<Order> _value_iterable; typedef ::better_enums::_Iterable<const char*> _name_iterable; typedef _value_iterable::iterator _value_iterator; typedef _name_iterable::iterator _name_iterator; constexpr static const std::size_t _size_constant = 192; constexpr static std::size_t _size() { return _size_constant; } constexpr static const char* _name(); constexpr static _value_iterable _values(); static _name_iterable _names(); _integral _value; private: Order() : _value(0) { } private: explicit constexpr Order(const _integral &value) : _value(value) { } static int initialize(); constexpr static _optional_index _from_value_loop(_integral value, std::size_t index = 0); constexpr static _optional_index _from_string_loop(const char *name, std::size_t index = 0); constexpr static _optional_index _from_string_nocase_loop(const char *name, std::size_t index = 0); friend struct ::better_enums::_initialize_at_program_start<Order>; }; namespace better_enums_data_Order { static ::better_enums::_initialize_at_program_start<Order> _force_initialization; enum _PutNamesInThisScopeAlso { Die = 0, Stop = 1, Guard = 2, PlayerGuard = 3, TurretGuard = 4, BunkerGuard = 5, Move = 6, ReaverStop = 7, Attack1 = 8, Attack2 = 9, AttackUnit = 10, AttackFixedRange = 11, AttackTile = 12, Hover = 13, AttackMove = 14, InfestedCommandCenter = 15, UnusedNothing = 16, UnusedPowerup = 17, TowerGuard = 18, TowerAttack = 19, VultureMine = 20, StayInRange = 21, TurretAttack = 22, Nothing = 23, Unused_24 = 24, DroneStartBuild = 25, DroneBuild = 26, CastInfestation = 27, MoveToInfest = 28, InfestingCommandCenter = 29, PlaceBuilding = 30, PlaceProtossBuilding = 31, CreateProtossBuilding = 32, ConstructingBuilding = 33, Repair = 34, MoveToRepair = 35, PlaceAddon = 36, BuildAddon = 37, Train = 38, RallyPointUnit = 39, RallyPointTile = 40, ZergBirth = 41, ZergUnitMorph = 42, ZergBuildingMorph = 43, IncompleteBuilding = 44, IncompleteMorphing = 45, BuildNydusExit = 46, EnterNydusCanal = 47, IncompleteWarping = 48, Follow = 49, Carrier = 50, ReaverCarrierMove = 51, CarrierStop = 52, CarrierAttack = 53, CarrierMoveToAttack = 54, CarrierIgnore2 = 55, CarrierFight = 56, CarrierHoldPosition = 57, Reaver = 58, ReaverAttack = 59, ReaverMoveToAttack = 60, ReaverFight = 61, ReaverHoldPosition = 62, TrainFighter = 63, InterceptorAttack = 64, ScarabAttack = 65, RechargeShieldsUnit = 66, RechargeShieldsBattery = 67, ShieldBattery = 68, InterceptorReturn = 69, DroneLand = 70, BuildingLand = 71, BuildingLiftOff = 72, DroneLiftOff = 73, LiftingOff = 74, ResearchTech = 75, Upgrade = 76, Larva = 77, SpawningLarva = 78, Harvest1 = 79, Harvest2 = 80, MoveToGas = 81, WaitForGas = 82, HarvestGas = 83, ReturnGas = 84, MoveToMinerals = 85, WaitForMinerals = 86, MiningMinerals = 87, Harvest3 = 88, Harvest4 = 89, ReturnMinerals = 90, Interrupted = 91, EnterTransport = 92, PickupIdle = 93, PickupTransport = 94, PickupBunker = 95, Pickup4 = 96, PowerupIdle = 97, Sieging = 98, Unsieging = 99, WatchTarget = 100, InitCreepGrowth = 101, SpreadCreep = 102, StoppingCreepGrowth = 103, GuardianAspect = 104, ArchonWarp = 105, CompletingArchonSummon = 106, HoldPosition = 107, QueenHoldPosition = 108, Cloak = 109, Decloak = 110, Unload = 111, MoveUnload = 112, FireYamatoGun = 113, MoveToFireYamatoGun = 114, CastLockdown = 115, Burrowing = 116, Burrowed = 117, Unburrowing = 118, CastDarkSwarm = 119, CastParasite = 120, CastSpawnBroodlings = 121, CastEMPShockwave = 122, NukeWait = 123, NukeTrain = 124, NukeLaunch = 125, NukePaint = 126, NukeUnit = 127, CastNuclearStrike = 128, NukeTrack = 129, InitializeArbiter = 130, CloakNearbyUnits = 131, PlaceMine = 132, RightClickAction = 133, SuicideUnit = 134, SuicideLocation = 135, SuicideHoldPosition = 136, CastRecall = 137, Teleport = 138, CastScannerSweep = 139, Scanner = 140, CastDefensiveMatrix = 141, CastPsionicStorm = 142, CastIrradiate = 143, CastPlague = 144, CastConsume = 145, CastEnsnare = 146, CastStasisField = 147, CastHallucination = 148, Hallucination2 = 149, ResetCollision = 150, ResetHarvestCollision = 151, Patrol = 152, CTFCOPInit = 153, CTFCOPStarted = 154, CTFCOP2 = 155, ComputerAI = 156, AtkMoveEP = 157, HarassMove = 158, AIPatrol = 159, GuardPost = 160, RescuePassive = 161, Neutral = 162, ComputerReturn = 163, InitializePsiProvider = 164, SelfDestructing = 165, Critter = 166, HiddenGun = 167, OpenDoor = 168, CloseDoor = 169, HideTrap = 170, RevealTrap = 171, EnableDoodad = 172, DisableDoodad = 173, WarpIn = 174, Medic = 175, MedicHeal = 176, HealMove = 177, MedicHoldPosition = 178, MedicHealToIdle = 179, CastRestoration = 180, CastDisruptionWeb = 181, CastMindControl = 182, DarkArchonMeld = 183, CastFeedback = 184, CastOpticalFlare = 185, CastMaelstrom = 186, JunkYardDog = 187, Fatal = 188, None = 189, Unknown = 190, MAX = 191 }; constexpr const Order _value_array[] = { ((::better_enums::_eat_assign<Order>)Order::Die = 0), ((::better_enums::_eat_assign<Order>)Order::Stop = 1), ((::better_enums::_eat_assign<Order>)Order::Guard = 2), ((::better_enums::_eat_assign<Order>)Order::PlayerGuard = 3), ((::better_enums::_eat_assign<Order>)Order::TurretGuard = 4), ((::better_enums::_eat_assign<Order>)Order::BunkerGuard = 5), ((::better_enums::_eat_assign<Order>)Order::Move = 6), ((::better_enums::_eat_assign<Order>)Order::ReaverStop = 7), ((::better_enums::_eat_assign<Order>)Order::Attack1 = 8), ((::better_enums::_eat_assign<Order>)Order::Attack2 = 9), ((::better_enums::_eat_assign<Order>)Order::AttackUnit = 10), ((::better_enums::_eat_assign<Order>)Order::AttackFixedRange = 11), ((::better_enums::_eat_assign<Order>)Order::AttackTile = 12), ((::better_enums::_eat_assign<Order>)Order::Hover = 13), ((::better_enums::_eat_assign<Order>)Order::AttackMove = 14), ((::better_enums::_eat_assign<Order>)Order::InfestedCommandCenter = 15), ((::better_enums::_eat_assign<Order>)Order::UnusedNothing = 16), ((::better_enums::_eat_assign<Order>)Order::UnusedPowerup = 17), ((::better_enums::_eat_assign<Order>)Order::TowerGuard = 18), ((::better_enums::_eat_assign<Order>)Order::TowerAttack = 19), ((::better_enums::_eat_assign<Order>)Order::VultureMine = 20), ((::better_enums::_eat_assign<Order>)Order::StayInRange = 21), ((::better_enums::_eat_assign<Order>)Order::TurretAttack = 22), ((::better_enums::_eat_assign<Order>)Order::Nothing = 23), ((::better_enums::_eat_assign<Order>)Order::Unused_24 = 24), ((::better_enums::_eat_assign<Order>)Order::DroneStartBuild = 25), ((::better_enums::_eat_assign<Order>)Order::DroneBuild = 26), ((::better_enums::_eat_assign<Order>)Order::CastInfestation = 27), ((::better_enums::_eat_assign<Order>)Order::MoveToInfest = 28), ((::better_enums::_eat_assign<Order>)Order::InfestingCommandCenter = 29), ((::better_enums::_eat_assign<Order>)Order::PlaceBuilding = 30), ((::better_enums::_eat_assign<Order>)Order::PlaceProtossBuilding = 31), ((::better_enums::_eat_assign<Order>)Order::CreateProtossBuilding = 32), ((::better_enums::_eat_assign<Order>)Order::ConstructingBuilding = 33), ((::better_enums::_eat_assign<Order>)Order::Repair = 34), ((::better_enums::_eat_assign<Order>)Order::MoveToRepair = 35), ((::better_enums::_eat_assign<Order>)Order::PlaceAddon = 36), ((::better_enums::_eat_assign<Order>)Order::BuildAddon = 37), ((::better_enums::_eat_assign<Order>)Order::Train = 38), ((::better_enums::_eat_assign<Order>)Order::RallyPointUnit = 39), ((::better_enums::_eat_assign<Order>)Order::RallyPointTile = 40), ((::better_enums::_eat_assign<Order>)Order::ZergBirth = 41), ((::better_enums::_eat_assign<Order>)Order::ZergUnitMorph = 42), ((::better_enums::_eat_assign<Order>)Order::ZergBuildingMorph = 43), ((::better_enums::_eat_assign<Order>)Order::IncompleteBuilding = 44), ((::better_enums::_eat_assign<Order>)Order::IncompleteMorphing = 45), ((::better_enums::_eat_assign<Order>)Order::BuildNydusExit = 46), ((::better_enums::_eat_assign<Order>)Order::EnterNydusCanal = 47), ((::better_enums::_eat_assign<Order>)Order::IncompleteWarping = 48), ((::better_enums::_eat_assign<Order>)Order::Follow = 49), ((::better_enums::_eat_assign<Order>)Order::Carrier = 50), ((::better_enums::_eat_assign<Order>)Order::ReaverCarrierMove = 51), ((::better_enums::_eat_assign<Order>)Order::CarrierStop = 52), ((::better_enums::_eat_assign<Order>)Order::CarrierAttack = 53), ((::better_enums::_eat_assign<Order>)Order::CarrierMoveToAttack = 54), ((::better_enums::_eat_assign<Order>)Order::CarrierIgnore2 = 55), ((::better_enums::_eat_assign<Order>)Order::CarrierFight = 56), ((::better_enums::_eat_assign<Order>)Order::CarrierHoldPosition = 57), ((::better_enums::_eat_assign<Order>)Order::Reaver = 58), ((::better_enums::_eat_assign<Order>)Order::ReaverAttack = 59), ((::better_enums::_eat_assign<Order>)Order::ReaverMoveToAttack = 60), ((::better_enums::_eat_assign<Order>)Order::ReaverFight = 61), ((::better_enums::_eat_assign<Order>)Order::ReaverHoldPosition = 62), ((::better_enums::_eat_assign<Order>)Order::TrainFighter = 63), ((::better_enums::_eat_assign<Order>)Order::InterceptorAttack = 64), ((::better_enums::_eat_assign<Order>)Order::ScarabAttack = 65), ((::better_enums::_eat_assign<Order>)Order::RechargeShieldsUnit = 66), ((::better_enums::_eat_assign<Order>)Order::RechargeShieldsBattery = 67), ((::better_enums::_eat_assign<Order>)Order::ShieldBattery = 68), ((::better_enums::_eat_assign<Order>)Order::InterceptorReturn = 69), ((::better_enums::_eat_assign<Order>)Order::DroneLand = 70), ((::better_enums::_eat_assign<Order>)Order::BuildingLand = 71), ((::better_enums::_eat_assign<Order>)Order::BuildingLiftOff = 72), ((::better_enums::_eat_assign<Order>)Order::DroneLiftOff = 73), ((::better_enums::_eat_assign<Order>)Order::LiftingOff = 74), ((::better_enums::_eat_assign<Order>)Order::ResearchTech = 75), ((::better_enums::_eat_assign<Order>)Order::Upgrade = 76), ((::better_enums::_eat_assign<Order>)Order::Larva = 77), ((::better_enums::_eat_assign<Order>)Order::SpawningLarva = 78), ((::better_enums::_eat_assign<Order>)Order::Harvest1 = 79), ((::better_enums::_eat_assign<Order>)Order::Harvest2 = 80), ((::better_enums::_eat_assign<Order>)Order::MoveToGas = 81), ((::better_enums::_eat_assign<Order>)Order::WaitForGas = 82), ((::better_enums::_eat_assign<Order>)Order::HarvestGas = 83), ((::better_enums::_eat_assign<Order>)Order::ReturnGas = 84), ((::better_enums::_eat_assign<Order>)Order::MoveToMinerals = 85), ((::better_enums::_eat_assign<Order>)Order::WaitForMinerals = 86), ((::better_enums::_eat_assign<Order>)Order::MiningMinerals = 87), ((::better_enums::_eat_assign<Order>)Order::Harvest3 = 88), ((::better_enums::_eat_assign<Order>)Order::Harvest4 = 89), ((::better_enums::_eat_assign<Order>)Order::ReturnMinerals = 90), ((::better_enums::_eat_assign<Order>)Order::Interrupted = 91), ((::better_enums::_eat_assign<Order>)Order::EnterTransport = 92), ((::better_enums::_eat_assign<Order>)Order::PickupIdle = 93), ((::better_enums::_eat_assign<Order>)Order::PickupTransport = 94), ((::better_enums::_eat_assign<Order>)Order::PickupBunker = 95), ((::better_enums::_eat_assign<Order>)Order::Pickup4 = 96), ((::better_enums::_eat_assign<Order>)Order::PowerupIdle = 97), ((::better_enums::_eat_assign<Order>)Order::Sieging = 98), ((::better_enums::_eat_assign<Order>)Order::Unsieging = 99), ((::better_enums::_eat_assign<Order>)Order::WatchTarget = 100), ((::better_enums::_eat_assign<Order>)Order::InitCreepGrowth = 101), ((::better_enums::_eat_assign<Order>)Order::SpreadCreep = 102), ((::better_enums::_eat_assign<Order>)Order::StoppingCreepGrowth = 103), ((::better_enums::_eat_assign<Order>)Order::GuardianAspect = 104), ((::better_enums::_eat_assign<Order>)Order::ArchonWarp = 105), ((::better_enums::_eat_assign<Order>)Order::CompletingArchonSummon = 106), ((::better_enums::_eat_assign<Order>)Order::HoldPosition = 107), ((::better_enums::_eat_assign<Order>)Order::QueenHoldPosition = 108), ((::better_enums::_eat_assign<Order>)Order::Cloak = 109), ((::better_enums::_eat_assign<Order>)Order::Decloak = 110), ((::better_enums::_eat_assign<Order>)Order::Unload = 111), ((::better_enums::_eat_assign<Order>)Order::MoveUnload = 112), ((::better_enums::_eat_assign<Order>)Order::FireYamatoGun = 113), ((::better_enums::_eat_assign<Order>)Order::MoveToFireYamatoGun = 114), ((::better_enums::_eat_assign<Order>)Order::CastLockdown = 115), ((::better_enums::_eat_assign<Order>)Order::Burrowing = 116), ((::better_enums::_eat_assign<Order>)Order::Burrowed = 117), ((::better_enums::_eat_assign<Order>)Order::Unburrowing = 118), ((::better_enums::_eat_assign<Order>)Order::CastDarkSwarm = 119), ((::better_enums::_eat_assign<Order>)Order::CastParasite = 120), ((::better_enums::_eat_assign<Order>)Order::CastSpawnBroodlings = 121), ((::better_enums::_eat_assign<Order>)Order::CastEMPShockwave = 122), ((::better_enums::_eat_assign<Order>)Order::NukeWait = 123), ((::better_enums::_eat_assign<Order>)Order::NukeTrain = 124), ((::better_enums::_eat_assign<Order>)Order::NukeLaunch = 125), ((::better_enums::_eat_assign<Order>)Order::NukePaint = 126), ((::better_enums::_eat_assign<Order>)Order::NukeUnit = 127), ((::better_enums::_eat_assign<Order>)Order::CastNuclearStrike = 128), ((::better_enums::_eat_assign<Order>)Order::NukeTrack = 129), ((::better_enums::_eat_assign<Order>)Order::InitializeArbiter = 130), ((::better_enums::_eat_assign<Order>)Order::CloakNearbyUnits = 131), ((::better_enums::_eat_assign<Order>)Order::PlaceMine = 132), ((::better_enums::_eat_assign<Order>)Order::RightClickAction = 133), ((::better_enums::_eat_assign<Order>)Order::SuicideUnit = 134), ((::better_enums::_eat_assign<Order>)Order::SuicideLocation = 135), ((::better_enums::_eat_assign<Order>)Order::SuicideHoldPosition = 136), ((::better_enums::_eat_assign<Order>)Order::CastRecall = 137), ((::better_enums::_eat_assign<Order>)Order::Teleport = 138), ((::better_enums::_eat_assign<Order>)Order::CastScannerSweep = 139), ((::better_enums::_eat_assign<Order>)Order::Scanner = 140), ((::better_enums::_eat_assign<Order>)Order::CastDefensiveMatrix = 141), ((::better_enums::_eat_assign<Order>)Order::CastPsionicStorm = 142), ((::better_enums::_eat_assign<Order>)Order::CastIrradiate = 143), ((::better_enums::_eat_assign<Order>)Order::CastPlague = 144), ((::better_enums::_eat_assign<Order>)Order::CastConsume = 145), ((::better_enums::_eat_assign<Order>)Order::CastEnsnare = 146), ((::better_enums::_eat_assign<Order>)Order::CastStasisField = 147), ((::better_enums::_eat_assign<Order>)Order::CastHallucination = 148), ((::better_enums::_eat_assign<Order>)Order::Hallucination2 = 149), ((::better_enums::_eat_assign<Order>)Order::ResetCollision = 150), ((::better_enums::_eat_assign<Order>)Order::ResetHarvestCollision = 151), ((::better_enums::_eat_assign<Order>)Order::Patrol = 152), ((::better_enums::_eat_assign<Order>)Order::CTFCOPInit = 153), ((::better_enums::_eat_assign<Order>)Order::CTFCOPStarted = 154), ((::better_enums::_eat_assign<Order>)Order::CTFCOP2 = 155), ((::better_enums::_eat_assign<Order>)Order::ComputerAI = 156), ((::better_enums::_eat_assign<Order>)Order::AtkMoveEP = 157), ((::better_enums::_eat_assign<Order>)Order::HarassMove = 158), ((::better_enums::_eat_assign<Order>)Order::AIPatrol = 159), ((::better_enums::_eat_assign<Order>)Order::GuardPost = 160), ((::better_enums::_eat_assign<Order>)Order::RescuePassive = 161), ((::better_enums::_eat_assign<Order>)Order::Neutral = 162), ((::better_enums::_eat_assign<Order>)Order::ComputerReturn = 163), ((::better_enums::_eat_assign<Order>)Order::InitializePsiProvider = 164), ((::better_enums::_eat_assign<Order>)Order::SelfDestructing = 165), ((::better_enums::_eat_assign<Order>)Order::Critter = 166), ((::better_enums::_eat_assign<Order>)Order::HiddenGun = 167), ((::better_enums::_eat_assign<Order>)Order::OpenDoor = 168), ((::better_enums::_eat_assign<Order>)Order::CloseDoor = 169), ((::better_enums::_eat_assign<Order>)Order::HideTrap = 170), ((::better_enums::_eat_assign<Order>)Order::RevealTrap = 171), ((::better_enums::_eat_assign<Order>)Order::EnableDoodad = 172), ((::better_enums::_eat_assign<Order>)Order::DisableDoodad = 173), ((::better_enums::_eat_assign<Order>)Order::WarpIn = 174), ((::better_enums::_eat_assign<Order>)Order::Medic = 175), ((::better_enums::_eat_assign<Order>)Order::MedicHeal = 176), ((::better_enums::_eat_assign<Order>)Order::HealMove = 177), ((::better_enums::_eat_assign<Order>)Order::MedicHoldPosition = 178), ((::better_enums::_eat_assign<Order>)Order::MedicHealToIdle = 179), ((::better_enums::_eat_assign<Order>)Order::CastRestoration = 180), ((::better_enums::_eat_assign<Order>)Order::CastDisruptionWeb = 181), ((::better_enums::_eat_assign<Order>)Order::CastMindControl = 182), ((::better_enums::_eat_assign<Order>)Order::DarkArchonMeld = 183), ((::better_enums::_eat_assign<Order>)Order::CastFeedback = 184), ((::better_enums::_eat_assign<Order>)Order::CastOpticalFlare = 185), ((::better_enums::_eat_assign<Order>)Order::CastMaelstrom = 186), ((::better_enums::_eat_assign<Order>)Order::JunkYardDog = 187), ((::better_enums::_eat_assign<Order>)Order::Fatal = 188), ((::better_enums::_eat_assign<Order>)Order::None = 189), ((::better_enums::_eat_assign<Order>)Order::Unknown = 190), ((::better_enums::_eat_assign<Order>)Order::MAX = 191), }; constexpr const char *_the_raw_names[] = { "Die = 0", "Stop = 1", "Guard = 2", "PlayerGuard = 3", "TurretGuard = 4", "BunkerGuard = 5", "Move = 6", "ReaverStop = 7", "Attack1 = 8", "Attack2 = 9", "AttackUnit = 10", "AttackFixedRange = 11", "AttackTile = 12", "Hover = 13", "AttackMove = 14", "InfestedCommandCenter = 15", "UnusedNothing = 16", "UnusedPowerup = 17", "TowerGuard = 18", "TowerAttack = 19", "VultureMine = 20", "StayInRange = 21", "TurretAttack = 22", "Nothing = 23", "Unused_24 = 24", "DroneStartBuild = 25", "DroneBuild = 26", "CastInfestation = 27", "MoveToInfest = 28", "InfestingCommandCenter = 29", "PlaceBuilding = 30", "PlaceProtossBuilding = 31", "CreateProtossBuilding = 32", "ConstructingBuilding = 33", "Repair = 34", "MoveToRepair = 35", "PlaceAddon = 36", "BuildAddon = 37", "Train = 38", "RallyPointUnit = 39", "RallyPointTile = 40", "ZergBirth = 41", "ZergUnitMorph = 42", "ZergBuildingMorph = 43", "IncompleteBuilding = 44", "IncompleteMorphing = 45", "BuildNydusExit = 46", "EnterNydusCanal = 47", "IncompleteWarping = 48", "Follow = 49", "Carrier = 50", "ReaverCarrierMove = 51", "CarrierStop = 52", "CarrierAttack = 53", "CarrierMoveToAttack = 54", "CarrierIgnore2 = 55", "CarrierFight = 56", "CarrierHoldPosition = 57", "Reaver = 58", "ReaverAttack = 59", "ReaverMoveToAttack = 60", "ReaverFight = 61", "ReaverHoldPosition = 62", "TrainFighter = 63", "InterceptorAttack = 64", "ScarabAttack = 65", "RechargeShieldsUnit = 66", "RechargeShieldsBattery = 67", "ShieldBattery = 68", "InterceptorReturn = 69", "DroneLand = 70", "BuildingLand = 71", "BuildingLiftOff = 72", "DroneLiftOff = 73", "LiftingOff = 74", "ResearchTech = 75", "Upgrade = 76", "Larva = 77", "SpawningLarva = 78", "Harvest1 = 79", "Harvest2 = 80", "MoveToGas = 81", "WaitForGas = 82", "HarvestGas = 83", "ReturnGas = 84", "MoveToMinerals = 85", "WaitForMinerals = 86", "MiningMinerals = 87", "Harvest3 = 88", "Harvest4 = 89", "ReturnMinerals = 90", "Interrupted = 91", "EnterTransport = 92", "PickupIdle = 93", "PickupTransport = 94", "PickupBunker = 95", "Pickup4 = 96", "PowerupIdle = 97", "Sieging = 98", "Unsieging = 99", "WatchTarget = 100", "InitCreepGrowth = 101", "SpreadCreep = 102", "StoppingCreepGrowth = 103", "GuardianAspect = 104", "ArchonWarp = 105", "CompletingArchonSummon = 106", "HoldPosition = 107", "QueenHoldPosition = 108", "Cloak = 109", "Decloak = 110", "Unload = 111", "MoveUnload = 112", "FireYamatoGun = 113", "MoveToFireYamatoGun = 114", "CastLockdown = 115", "Burrowing = 116", "Burrowed = 117", "Unburrowing = 118", "CastDarkSwarm = 119", "CastParasite = 120", "CastSpawnBroodlings = 121", "CastEMPShockwave = 122", "NukeWait = 123", "NukeTrain = 124", "NukeLaunch = 125", "NukePaint = 126", "NukeUnit = 127", "CastNuclearStrike = 128", "NukeTrack = 129", "InitializeArbiter = 130", "CloakNearbyUnits = 131", "PlaceMine = 132", "RightClickAction = 133", "SuicideUnit = 134", "SuicideLocation = 135", "SuicideHoldPosition = 136", "CastRecall = 137", "Teleport = 138", "CastScannerSweep = 139", "Scanner = 140", "CastDefensiveMatrix = 141", "CastPsionicStorm = 142", "CastIrradiate = 143", "CastPlague = 144", "CastConsume = 145", "CastEnsnare = 146", "CastStasisField = 147", "CastHallucination = 148", "Hallucination2 = 149", "ResetCollision = 150", "ResetHarvestCollision = 151", "Patrol = 152", "CTFCOPInit = 153", "CTFCOPStarted = 154", "CTFCOP2 = 155", "ComputerAI = 156", "AtkMoveEP = 157", "HarassMove = 158", "AIPatrol = 159", "GuardPost = 160", "RescuePassive = 161", "Neutral = 162", "ComputerReturn = 163", "InitializePsiProvider = 164", "SelfDestructing = 165", "Critter = 166", "HiddenGun = 167", "OpenDoor = 168", "CloseDoor = 169", "HideTrap = 170", "RevealTrap = 171", "EnableDoodad = 172", "DisableDoodad = 173", "WarpIn = 174", "Medic = 175", "MedicHeal = 176", "HealMove = 177", "MedicHoldPosition = 178", "MedicHealToIdle = 179", "CastRestoration = 180", "CastDisruptionWeb = 181", "CastMindControl = 182", "DarkArchonMeld = 183", "CastFeedback = 184", "CastOpticalFlare = 185", "CastMaelstrom = 186", "JunkYardDog = 187", "Fatal = 188", "None = 189", "Unknown = 190", "MAX = 191", }; constexpr const char * const * _raw_names() { return _the_raw_names; } inline char* _name_storage() { static char storage[] = "Die = 0" "," "Stop = 1" "," "Guard = 2" "," "PlayerGuard = 3" "," "TurretGuard = 4" "," "BunkerGuard = 5" "," "Move = 6" "," "ReaverStop = 7" "," "Attack1 = 8" "," "Attack2 = 9" "," "AttackUnit = 10" "," "AttackFixedRange = 11" "," "AttackTile = 12" "," "Hover = 13" "," "AttackMove = 14" "," "InfestedCommandCenter = 15" "," "UnusedNothing = 16" "," "UnusedPowerup = 17" "," "TowerGuard = 18" "," "TowerAttack = 19" "," "VultureMine = 20" "," "StayInRange = 21" "," "TurretAttack = 22" "," "Nothing = 23" "," "Unused_24 = 24" "," "DroneStartBuild = 25" "," "DroneBuild = 26" "," "CastInfestation = 27" "," "MoveToInfest = 28" "," "InfestingCommandCenter = 29" "," "PlaceBuilding = 30" "," "PlaceProtossBuilding = 31" "," "CreateProtossBuilding = 32" "," "ConstructingBuilding = 33" "," "Repair = 34" "," "MoveToRepair = 35" "," "PlaceAddon = 36" "," "BuildAddon = 37" "," "Train = 38" "," "RallyPointUnit = 39" "," "RallyPointTile = 40" "," "ZergBirth = 41" "," "ZergUnitMorph = 42" "," "ZergBuildingMorph = 43" "," "IncompleteBuilding = 44" "," "IncompleteMorphing = 45" "," "BuildNydusExit = 46" "," "EnterNydusCanal = 47" "," "IncompleteWarping = 48" "," "Follow = 49" "," "Carrier = 50" "," "ReaverCarrierMove = 51" "," "CarrierStop = 52" "," "CarrierAttack = 53" "," "CarrierMoveToAttack = 54" "," "CarrierIgnore2 = 55" "," "CarrierFight = 56" "," "CarrierHoldPosition = 57" "," "Reaver = 58" "," "ReaverAttack = 59" "," "ReaverMoveToAttack = 60" "," "ReaverFight = 61" "," "ReaverHoldPosition = 62" "," "TrainFighter = 63" "," "InterceptorAttack = 64" "," "ScarabAttack = 65" "," "RechargeShieldsUnit = 66" "," "RechargeShieldsBattery = 67" "," "ShieldBattery = 68" "," "InterceptorReturn = 69" "," "DroneLand = 70" "," "BuildingLand = 71" "," "BuildingLiftOff = 72" "," "DroneLiftOff = 73" "," "LiftingOff = 74" "," "ResearchTech = 75" "," "Upgrade = 76" "," "Larva = 77" "," "SpawningLarva = 78" "," "Harvest1 = 79" "," "Harvest2 = 80" "," "MoveToGas = 81" "," "WaitForGas = 82" "," "HarvestGas = 83" "," "ReturnGas = 84" "," "MoveToMinerals = 85" "," "WaitForMinerals = 86" "," "MiningMinerals = 87" "," "Harvest3 = 88" "," "Harvest4 = 89" "," "ReturnMinerals = 90" "," "Interrupted = 91" "," "EnterTransport = 92" "," "PickupIdle = 93" "," "PickupTransport = 94" "," "PickupBunker = 95" "," "Pickup4 = 96" "," "PowerupIdle = 97" "," "Sieging = 98" "," "Unsieging = 99" "," "WatchTarget = 100" "," "InitCreepGrowth = 101" "," "SpreadCreep = 102" "," "StoppingCreepGrowth = 103" "," "GuardianAspect = 104" "," "ArchonWarp = 105" "," "CompletingArchonSummon = 106" "," "HoldPosition = 107" "," "QueenHoldPosition = 108" "," "Cloak = 109" "," "Decloak = 110" "," "Unload = 111" "," "MoveUnload = 112" "," "FireYamatoGun = 113" "," "MoveToFireYamatoGun = 114" "," "CastLockdown = 115" "," "Burrowing = 116" "," "Burrowed = 117" "," "Unburrowing = 118" "," "CastDarkSwarm = 119" "," "CastParasite = 120" "," "CastSpawnBroodlings = 121" "," "CastEMPShockwave = 122" "," "NukeWait = 123" "," "NukeTrain = 124" "," "NukeLaunch = 125" "," "NukePaint = 126" "," "NukeUnit = 127" "," "CastNuclearStrike = 128" "," "NukeTrack = 129" "," "InitializeArbiter = 130" "," "CloakNearbyUnits = 131" "," "PlaceMine = 132" "," "RightClickAction = 133" "," "SuicideUnit = 134" "," "SuicideLocation = 135" "," "SuicideHoldPosition = 136" "," "CastRecall = 137" "," "Teleport = 138" "," "CastScannerSweep = 139" "," "Scanner = 140" "," "CastDefensiveMatrix = 141" "," "CastPsionicStorm = 142" "," "CastIrradiate = 143" "," "CastPlague = 144" "," "CastConsume = 145" "," "CastEnsnare = 146" "," "CastStasisField = 147" "," "CastHallucination = 148" "," "Hallucination2 = 149" "," "ResetCollision = 150" "," "ResetHarvestCollision = 151" "," "Patrol = 152" "," "CTFCOPInit = 153" "," "CTFCOPStarted = 154" "," "CTFCOP2 = 155" "," "ComputerAI = 156" "," "AtkMoveEP = 157" "," "HarassMove = 158" "," "AIPatrol = 159" "," "GuardPost = 160" "," "RescuePassive = 161" "," "Neutral = 162" "," "ComputerReturn = 163" "," "InitializePsiProvider = 164" "," "SelfDestructing = 165" "," "Critter = 166" "," "HiddenGun = 167" "," "OpenDoor = 168" "," "CloseDoor = 169" "," "HideTrap = 170" "," "RevealTrap = 171" "," "EnableDoodad = 172" "," "DisableDoodad = 173" "," "WarpIn = 174" "," "Medic = 175" "," "MedicHeal = 176" "," "HealMove = 177" "," "MedicHoldPosition = 178" "," "MedicHealToIdle = 179" "," "CastRestoration = 180" "," "CastDisruptionWeb = 181" "," "CastMindControl = 182" "," "DarkArchonMeld = 183" "," "CastFeedback = 184" "," "CastOpticalFlare = 185" "," "CastMaelstrom = 186" "," "JunkYardDog = 187" "," "Fatal = 188" "," "None = 189" "," "Unknown = 190" "," "MAX = 191" ","; return storage; } inline const char** _name_array() { static const char *value[Order::_size_constant]; return value; } inline bool& _initialized() { static bool value = false; return value; } }  constexpr inline const Order operator +(Order::_enumerated enumerated) { return static_cast<Order>(enumerated); } constexpr inline Order::_optional_index Order::_from_value_loop(Order::_integral value, std::size_t index) { return index == _size() ? _optional_index() : better_enums_data_Order::_value_array[index]._value == value ? _optional_index(index) : _from_value_loop(value, index + 1); } constexpr inline Order::_optional_index Order::_from_string_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match(better_enums_data_Order::_raw_names()[index], name) ? _optional_index(index) : _from_string_loop(name, index + 1); } constexpr inline Order::_optional_index Order::_from_string_nocase_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match_nocase(better_enums_data_Order::_raw_names()[index], name) ? _optional_index(index) : _from_string_nocase_loop(name, index + 1); } constexpr inline Order::_integral Order::_to_integral() const { return _integral(_value); } constexpr inline Order Order::_from_integral_unchecked(_integral value) { return static_cast<_enumerated>(value); } constexpr inline Order::_optional Order::_from_integral_nothrow(_integral value) { return ::better_enums::_map_index<Order>(better_enums_data_Order::_value_array, _from_value_loop(value)); } constexpr inline Order Order::_from_integral(_integral value) { return ::better_enums::_or_throw(_from_integral_nothrow(value), "Order" "::_from_integral: invalid argument"); } inline const char* Order::_to_string() const { return ::better_enums::_or_null(::better_enums::_map_index<const char*>(better_enums_data_Order::_name_array(), _from_value_loop(::better_enums::continue_with(initialize(), _value)))); } constexpr inline Order::_optional Order::_from_string_nothrow(const char *name) { return ::better_enums::_map_index<Order>(better_enums_data_Order::_value_array, _from_string_loop(name)); } constexpr inline Order Order::_from_string(const char *name) { return ::better_enums::_or_throw(_from_string_nothrow(name), "Order" "::_from_string: invalid argument"); } constexpr inline Order::_optional Order::_from_string_nocase_nothrow(const char *name) { return ::better_enums::_map_index<Order>(better_enums_data_Order::_value_array, _from_string_nocase_loop(name)); } constexpr inline Order Order::_from_string_nocase(const char *name) { return ::better_enums::_or_throw(_from_string_nocase_nothrow(name), "Order" "::_from_string_nocase: invalid argument"); } constexpr inline bool Order::_is_valid(_integral value) { return _from_value_loop(value); } constexpr inline bool Order::_is_valid(const char *name) { return _from_string_loop(name); } constexpr inline bool Order::_is_valid_nocase(const char *name) { return _from_string_nocase_loop(name); } constexpr inline const char* Order::_name() { return "Order"; } constexpr inline Order::_value_iterable Order::_values() { return _value_iterable(better_enums_data_Order::_value_array, _size()); } inline Order::_name_iterable Order::_names() { return _name_iterable(better_enums_data_Order::_name_array(), ::better_enums::continue_with(initialize(), _size())); } inline int Order::initialize() { if (better_enums_data_Order::_initialized()) return 0; ::better_enums::_trim_names(better_enums_data_Order::_raw_names(), better_enums_data_Order::_name_array(), better_enums_data_Order::_name_storage(), _size()); better_enums_data_Order::_initialized() = true; return 0; }  constexpr inline bool operator ==(const Order &a, const Order &b) { return a._to_integral() == b._to_integral(); }  constexpr inline bool operator !=(const Order &a, const Order &b) { return a._to_integral() != b._to_integral(); }  constexpr inline bool operator <(const Order &a, const Order &b) { return a._to_integral() < b._to_integral(); }  constexpr inline bool operator <=(const Order &a, const Order &b) { return a._to_integral() <= b._to_integral(); }  constexpr inline bool operator >(const Order &a, const Order &b) { return a._to_integral() > b._to_integral(); }  constexpr inline bool operator >=(const Order &a, const Order &b) { return a._to_integral() >= b._to_integral(); } template <typename Char, typename Traits> std::basic_ostream<Char, Traits>& operator <<(std::basic_ostream<Char, Traits>& stream, const Order &value) { return stream << value._to_string(); } template <typename Char, typename Traits> std::basic_istream<Char, Traits>& operator >>(std::basic_istream<Char, Traits>& stream, Order &value) { std::basic_string<Char, Traits> buffer; stream >> buffer; ::better_enums::optional<Order> converted = Order::_from_string_nothrow(buffer.c_str()); if (converted) value = *converted; else stream.setstate(std::basic_istream<Char, Traits>::failbit); return stream; }
#endif // TC_EXPAND_LARGE_ENUMS

BETTER_ENUM(
    UpgradeType,
    int,
    // corresponds to BWAPI::UpgradeTypes::Enum
    Terran_Infantry_Armor = 0,
    Terran_Vehicle_Plating = 1,
    Terran_Ship_Plating = 2,
    Zerg_Carapace = 3,
    Zerg_Flyer_Carapace = 4,
    Protoss_Ground_Armor = 5,
    Protoss_Air_Armor = 6,
    Terran_Infantry_Weapons = 7,
    Terran_Vehicle_Weapons = 8,
    Terran_Ship_Weapons = 9,
    Zerg_Melee_Attacks = 10,
    Zerg_Missile_Attacks = 11,
    Zerg_Flyer_Attacks = 12,
    Protoss_Ground_Weapons = 13,
    Protoss_Air_Weapons = 14,
    Protoss_Plasma_Shields = 15,
    U_238_Shells = 16,
    Ion_Thrusters = 17,
    Titan_Reactor = 19,
    Ocular_Implants = 20,
    Moebius_Reactor = 21,
    Apollo_Reactor = 22,
    Colossus_Reactor = 23,
    Ventral_Sacs = 24,
    Antennae = 25,
    Pneumatized_Carapace = 26,
    Metabolic_Boost = 27,
    Adrenal_Glands = 28,
    Muscular_Augments = 29,
    Grooved_Spines = 30,
    Gamete_Meiosis = 31,
    Metasynaptic_Node = 32,
    Singularity_Charge = 33,
    Leg_Enhancements = 34,
    Scarab_Damage = 35,
    Reaver_Capacity = 36,
    Gravitic_Drive = 37,
    Sensor_Array = 38,
    Gravitic_Boosters = 39,
    Khaydarin_Amulet = 40,
    Apial_Sensors = 41,
    Gravitic_Thrusters = 42,
    Carrier_Capacity = 43,
    Khaydarin_Core = 44,
    Argus_Jewel = 47,
    Argus_Talisman = 49,
    Caduceus_Reactor = 51,
    Chitinous_Plating = 52,
    Anabolic_Synthesis = 53,
    Charon_Boosters = 54,
    Upgrade_60 = 60,
    None = 61,
    Unknow = 62,
    MAX = 63)

#ifdef TC_EXPAND_LARGE_ENUMS
// Be sure to update the expanded version below when changing this
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
#else // TC_EXPAND_LARGE_ENUMS
namespace better_enums_data_TechType {} class TechType { private: typedef ::better_enums::optional<TechType> _optional; typedef ::better_enums::optional<std::size_t> _optional_index; public: typedef int _integral; enum _enumerated : int { Stim_Packs = 0, Lockdown = 1, EMP_Shockwave = 2, Spider_Mines = 3, Scanner_Sweep = 4, Tank_Siege_Mode = 5, Defensive_Matrix = 6, Irradiate = 7, Yamato_Gun = 8, Cloaking_Field = 9, Personnel_Cloaking = 10, Burrowing = 11, Infestation = 12, Spawn_Broodlings = 13, Dark_Swarm = 14, Plague = 15, Consume = 16, Ensnare = 17, Parasite = 18, Psionic_Storm = 19, Hallucination = 20, Recall = 21, Stasis_Field = 22, Archon_Warp = 23, Restoration = 24, Disruption_Web = 25, Unused_26 = 26, Mind_Control = 27, Dark_Archon_Meld = 28, Feedback = 29, Optical_Flare = 30, Maelstrom = 31, Lurker_Aspect = 32, Unused_33 = 33, Healing = 34, None = 44, Nuclear_Strike = 45, Unknown = 46, MAX = 47 }; constexpr TechType(_enumerated value) : _value(value) { } constexpr operator _enumerated() const { return _enumerated(_value); } constexpr _integral _to_integral() const; constexpr static TechType _from_integral(_integral value); constexpr static TechType _from_integral_unchecked(_integral value); constexpr static _optional _from_integral_nothrow(_integral value); const char* _to_string() const; constexpr static TechType _from_string(const char *name); constexpr static _optional _from_string_nothrow(const char *name); constexpr static TechType _from_string_nocase(const char *name); constexpr static _optional _from_string_nocase_nothrow(const char *name); constexpr static bool _is_valid(_integral value); constexpr static bool _is_valid(const char *name); constexpr static bool _is_valid_nocase(const char *name); typedef ::better_enums::_Iterable<TechType> _value_iterable; typedef ::better_enums::_Iterable<const char*> _name_iterable; typedef _value_iterable::iterator _value_iterator; typedef _name_iterable::iterator _name_iterator; constexpr static const std::size_t _size_constant = 39; constexpr static std::size_t _size() { return _size_constant; } constexpr static const char* _name(); constexpr static _value_iterable _values(); static _name_iterable _names(); _integral _value; private: TechType() : _value(0) { } private: explicit constexpr TechType(const _integral &value) : _value(value) { } static int initialize(); constexpr static _optional_index _from_value_loop(_integral value, std::size_t index = 0); constexpr static _optional_index _from_string_loop(const char *name, std::size_t index = 0); constexpr static _optional_index _from_string_nocase_loop(const char *name, std::size_t index = 0); friend struct ::better_enums::_initialize_at_program_start<TechType>; }; namespace better_enums_data_TechType { static ::better_enums::_initialize_at_program_start<TechType> _force_initialization; enum _PutNamesInThisScopeAlso { Stim_Packs = 0, Lockdown = 1, EMP_Shockwave = 2, Spider_Mines = 3, Scanner_Sweep = 4, Tank_Siege_Mode = 5, Defensive_Matrix = 6, Irradiate = 7, Yamato_Gun = 8, Cloaking_Field = 9, Personnel_Cloaking = 10, Burrowing = 11, Infestation = 12, Spawn_Broodlings = 13, Dark_Swarm = 14, Plague = 15, Consume = 16, Ensnare = 17, Parasite = 18, Psionic_Storm = 19, Hallucination = 20, Recall = 21, Stasis_Field = 22, Archon_Warp = 23, Restoration = 24, Disruption_Web = 25, Unused_26 = 26, Mind_Control = 27, Dark_Archon_Meld = 28, Feedback = 29, Optical_Flare = 30, Maelstrom = 31, Lurker_Aspect = 32, Unused_33 = 33, Healing = 34, None = 44, Nuclear_Strike = 45, Unknown = 46, MAX = 47 }; constexpr const TechType _value_array[] = { ((::better_enums::_eat_assign<TechType>)TechType::Stim_Packs = 0), ((::better_enums::_eat_assign<TechType>)TechType::Lockdown = 1), ((::better_enums::_eat_assign<TechType>)TechType::EMP_Shockwave = 2), ((::better_enums::_eat_assign<TechType>)TechType::Spider_Mines = 3), ((::better_enums::_eat_assign<TechType>)TechType::Scanner_Sweep = 4), ((::better_enums::_eat_assign<TechType>)TechType::Tank_Siege_Mode = 5), ((::better_enums::_eat_assign<TechType>)TechType::Defensive_Matrix = 6), ((::better_enums::_eat_assign<TechType>)TechType::Irradiate = 7), ((::better_enums::_eat_assign<TechType>)TechType::Yamato_Gun = 8), ((::better_enums::_eat_assign<TechType>)TechType::Cloaking_Field = 9), ((::better_enums::_eat_assign<TechType>)TechType::Personnel_Cloaking = 10), ((::better_enums::_eat_assign<TechType>)TechType::Burrowing = 11), ((::better_enums::_eat_assign<TechType>)TechType::Infestation = 12), ((::better_enums::_eat_assign<TechType>)TechType::Spawn_Broodlings = 13), ((::better_enums::_eat_assign<TechType>)TechType::Dark_Swarm = 14), ((::better_enums::_eat_assign<TechType>)TechType::Plague = 15), ((::better_enums::_eat_assign<TechType>)TechType::Consume = 16), ((::better_enums::_eat_assign<TechType>)TechType::Ensnare = 17), ((::better_enums::_eat_assign<TechType>)TechType::Parasite = 18), ((::better_enums::_eat_assign<TechType>)TechType::Psionic_Storm = 19), ((::better_enums::_eat_assign<TechType>)TechType::Hallucination = 20), ((::better_enums::_eat_assign<TechType>)TechType::Recall = 21), ((::better_enums::_eat_assign<TechType>)TechType::Stasis_Field = 22), ((::better_enums::_eat_assign<TechType>)TechType::Archon_Warp = 23), ((::better_enums::_eat_assign<TechType>)TechType::Restoration = 24), ((::better_enums::_eat_assign<TechType>)TechType::Disruption_Web = 25), ((::better_enums::_eat_assign<TechType>)TechType::Unused_26 = 26), ((::better_enums::_eat_assign<TechType>)TechType::Mind_Control = 27), ((::better_enums::_eat_assign<TechType>)TechType::Dark_Archon_Meld = 28), ((::better_enums::_eat_assign<TechType>)TechType::Feedback = 29), ((::better_enums::_eat_assign<TechType>)TechType::Optical_Flare = 30), ((::better_enums::_eat_assign<TechType>)TechType::Maelstrom = 31), ((::better_enums::_eat_assign<TechType>)TechType::Lurker_Aspect = 32), ((::better_enums::_eat_assign<TechType>)TechType::Unused_33 = 33), ((::better_enums::_eat_assign<TechType>)TechType::Healing = 34), ((::better_enums::_eat_assign<TechType>)TechType::None = 44), ((::better_enums::_eat_assign<TechType>)TechType::Nuclear_Strike = 45), ((::better_enums::_eat_assign<TechType>)TechType::Unknown = 46), ((::better_enums::_eat_assign<TechType>)TechType::MAX = 47), }; constexpr const char *_the_raw_names[] = { "Stim_Packs = 0", "Lockdown = 1", "EMP_Shockwave = 2", "Spider_Mines = 3", "Scanner_Sweep = 4", "Tank_Siege_Mode = 5", "Defensive_Matrix = 6", "Irradiate = 7", "Yamato_Gun = 8", "Cloaking_Field = 9", "Personnel_Cloaking = 10", "Burrowing = 11", "Infestation = 12", "Spawn_Broodlings = 13", "Dark_Swarm = 14", "Plague = 15", "Consume = 16", "Ensnare = 17", "Parasite = 18", "Psionic_Storm = 19", "Hallucination = 20", "Recall = 21", "Stasis_Field = 22", "Archon_Warp = 23", "Restoration = 24", "Disruption_Web = 25", "Unused_26 = 26", "Mind_Control = 27", "Dark_Archon_Meld = 28", "Feedback = 29", "Optical_Flare = 30", "Maelstrom = 31", "Lurker_Aspect = 32", "Unused_33 = 33", "Healing = 34", "None = 44", "Nuclear_Strike = 45", "Unknown = 46", "MAX = 47", }; constexpr const char * const * _raw_names() { return _the_raw_names; } inline char* _name_storage() { static char storage[] = "Stim_Packs = 0" "," "Lockdown = 1" "," "EMP_Shockwave = 2" "," "Spider_Mines = 3" "," "Scanner_Sweep = 4" "," "Tank_Siege_Mode = 5" "," "Defensive_Matrix = 6" "," "Irradiate = 7" "," "Yamato_Gun = 8" "," "Cloaking_Field = 9" "," "Personnel_Cloaking = 10" "," "Burrowing = 11" "," "Infestation = 12" "," "Spawn_Broodlings = 13" "," "Dark_Swarm = 14" "," "Plague = 15" "," "Consume = 16" "," "Ensnare = 17" "," "Parasite = 18" "," "Psionic_Storm = 19" "," "Hallucination = 20" "," "Recall = 21" "," "Stasis_Field = 22" "," "Archon_Warp = 23" "," "Restoration = 24" "," "Disruption_Web = 25" "," "Unused_26 = 26" "," "Mind_Control = 27" "," "Dark_Archon_Meld = 28" "," "Feedback = 29" "," "Optical_Flare = 30" "," "Maelstrom = 31" "," "Lurker_Aspect = 32" "," "Unused_33 = 33" "," "Healing = 34" "," "None = 44" "," "Nuclear_Strike = 45" "," "Unknown = 46" "," "MAX = 47" ","; return storage; } inline const char** _name_array() { static const char *value[TechType::_size_constant]; return value; } inline bool& _initialized() { static bool value = false; return value; } }  constexpr inline const TechType operator +(TechType::_enumerated enumerated) { return static_cast<TechType>(enumerated); } constexpr inline TechType::_optional_index TechType::_from_value_loop(TechType::_integral value, std::size_t index) { return index == _size() ? _optional_index() : better_enums_data_TechType::_value_array[index]._value == value ? _optional_index(index) : _from_value_loop(value, index + 1); } constexpr inline TechType::_optional_index TechType::_from_string_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match(better_enums_data_TechType::_raw_names()[index], name) ? _optional_index(index) : _from_string_loop(name, index + 1); } constexpr inline TechType::_optional_index TechType::_from_string_nocase_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match_nocase(better_enums_data_TechType::_raw_names()[index], name) ? _optional_index(index) : _from_string_nocase_loop(name, index + 1); } constexpr inline TechType::_integral TechType::_to_integral() const { return _integral(_value); } constexpr inline TechType TechType::_from_integral_unchecked(_integral value) { return static_cast<_enumerated>(value); } constexpr inline TechType::_optional TechType::_from_integral_nothrow(_integral value) { return ::better_enums::_map_index<TechType>(better_enums_data_TechType::_value_array, _from_value_loop(value)); } constexpr inline TechType TechType::_from_integral(_integral value) { return ::better_enums::_or_throw(_from_integral_nothrow(value), "TechType" "::_from_integral: invalid argument"); } inline const char* TechType::_to_string() const { return ::better_enums::_or_null(::better_enums::_map_index<const char*>(better_enums_data_TechType::_name_array(), _from_value_loop(::better_enums::continue_with(initialize(), _value)))); } constexpr inline TechType::_optional TechType::_from_string_nothrow(const char *name) { return ::better_enums::_map_index<TechType>(better_enums_data_TechType::_value_array, _from_string_loop(name)); } constexpr inline TechType TechType::_from_string(const char *name) { return ::better_enums::_or_throw(_from_string_nothrow(name), "TechType" "::_from_string: invalid argument"); } constexpr inline TechType::_optional TechType::_from_string_nocase_nothrow(const char *name) { return ::better_enums::_map_index<TechType>(better_enums_data_TechType::_value_array, _from_string_nocase_loop(name)); } constexpr inline TechType TechType::_from_string_nocase(const char *name) { return ::better_enums::_or_throw(_from_string_nocase_nothrow(name), "TechType" "::_from_string_nocase: invalid argument"); } constexpr inline bool TechType::_is_valid(_integral value) { return _from_value_loop(value); } constexpr inline bool TechType::_is_valid(const char *name) { return _from_string_loop(name); } constexpr inline bool TechType::_is_valid_nocase(const char *name) { return _from_string_nocase_loop(name); } constexpr inline const char* TechType::_name() { return "TechType"; } constexpr inline TechType::_value_iterable TechType::_values() { return _value_iterable(better_enums_data_TechType::_value_array, _size()); } inline TechType::_name_iterable TechType::_names() { return _name_iterable(better_enums_data_TechType::_name_array(), ::better_enums::continue_with(initialize(), _size())); } inline int TechType::initialize() { if (better_enums_data_TechType::_initialized()) return 0; ::better_enums::_trim_names(better_enums_data_TechType::_raw_names(), better_enums_data_TechType::_name_array(), better_enums_data_TechType::_name_storage(), _size()); better_enums_data_TechType::_initialized() = true; return 0; }  constexpr inline bool operator ==(const TechType &a, const TechType &b) { return a._to_integral() == b._to_integral(); }  constexpr inline bool operator !=(const TechType &a, const TechType &b) { return a._to_integral() != b._to_integral(); }  constexpr inline bool operator <(const TechType &a, const TechType &b) { return a._to_integral() < b._to_integral(); }  constexpr inline bool operator <=(const TechType &a, const TechType &b) { return a._to_integral() <= b._to_integral(); }  constexpr inline bool operator >(const TechType &a, const TechType &b) { return a._to_integral() > b._to_integral(); }  constexpr inline bool operator >=(const TechType &a, const TechType &b) { return a._to_integral() >= b._to_integral(); } template <typename Char, typename Traits> std::basic_ostream<Char, Traits>& operator <<(std::basic_ostream<Char, Traits>& stream, const TechType &value) { return stream << value._to_string(); } template <typename Char, typename Traits> std::basic_istream<Char, Traits>& operator >>(std::basic_istream<Char, Traits>& stream, TechType &value) { std::basic_string<Char, Traits> buffer; stream >> buffer; ::better_enums::optional<TechType> converted = TechType::_from_string_nothrow(buffer.c_str()); if (converted) value = *converted; else stream.setstate(std::basic_istream<Char, Traits>::failbit); return stream; }
#endif // TC_EXPAND_LARGE_ENUMS

#ifndef TC_EXPAND_LARGE_ENUMS
// Be sure to update the expanded version below when changing this
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
#else // TC_EXPAND_LARGE_ENUMS
namespace better_enums_data_UnitType {} class UnitType { private: typedef ::better_enums::optional<UnitType> _optional; typedef ::better_enums::optional<std::size_t> _optional_index; public: typedef int _integral; enum _enumerated : int { Terran_Marine = 0, Terran_Ghost = 1, Terran_Vulture = 2, Terran_Goliath = 3, Terran_Siege_Tank_Tank_Mode = 5, Terran_SCV = 7, Terran_Wraith = 8, Terran_Science_Vessel = 9, Terran_Dropship = 11, Terran_Battlecruiser = 12, Terran_Vulture_Spider_Mine = 13, Terran_Nuclear_Missile = 14, Terran_Civilian = 15, Terran_Siege_Tank_Siege_Mode = 30, Terran_Firebat = 32, Spell_Scanner_Sweep = 33, Terran_Medic = 34, Zerg_Larva = 35, Zerg_Egg = 36, Zerg_Zergling = 37, Zerg_Hydralisk = 38, Zerg_Ultralisk = 39, Zerg_Broodling = 40, Zerg_Drone = 41, Zerg_Overlord = 42, Zerg_Mutalisk = 43, Zerg_Guardian = 44, Zerg_Queen = 45, Zerg_Defiler = 46, Zerg_Scourge = 47, Zerg_Infested_Terran = 50, Terran_Valkyrie = 58, Zerg_Cocoon = 59, Protoss_Corsair = 60, Protoss_Dark_Templar = 61, Zerg_Devourer = 62, Protoss_Dark_Archon = 63, Protoss_Probe = 64, Protoss_Zealot = 65, Protoss_Dragoon = 66, Protoss_High_Templar = 67, Protoss_Archon = 68, Protoss_Shuttle = 69, Protoss_Scout = 70, Protoss_Arbiter = 71, Protoss_Carrier = 72, Protoss_Interceptor = 73, Protoss_Reaver = 83, Protoss_Observer = 84, Protoss_Scarab = 85, Critter_Rhynadon = 89, Critter_Bengalaas = 90, Critter_Scantid = 93, Critter_Kakaru = 94, Critter_Ragnasaur = 95, Critter_Ursadon = 96, Zerg_Lurker_Egg = 97, Zerg_Lurker = 103, Spell_Disruption_Web = 105, Terran_Command_Center = 106, Terran_Comsat_Station = 107, Terran_Nuclear_Silo = 108, Terran_Supply_Depot = 109, Terran_Refinery = 110, Terran_Barracks = 111, Terran_Academy = 112, Terran_Factory = 113, Terran_Starport = 114, Terran_Control_Tower = 115, Terran_Science_Facility = 116, Terran_Covert_Ops = 117, Terran_Physics_Lab = 118, Terran_Machine_Shop = 120, Terran_Engineering_Bay = 122, Terran_Armory = 123, Terran_Missile_Turret = 124, Terran_Bunker = 125, Zerg_Infested_Command_Center = 130, Zerg_Hatchery = 131, Zerg_Lair = 132, Zerg_Hive = 133, Zerg_Nydus_Canal = 134, Zerg_Hydralisk_Den = 135, Zerg_Defiler_Mound = 136, Zerg_Greater_Spire = 137, Zerg_Queens_Nest = 138, Zerg_Evolution_Chamber = 139, Zerg_Ultralisk_Cavern = 140, Zerg_Spire = 141, Zerg_Spawning_Pool = 142, Zerg_Creep_Colony = 143, Zerg_Spore_Colony = 144, Zerg_Sunken_Colony = 146, Zerg_Extractor = 149, Protoss_Nexus = 154, Protoss_Robotics_Facility = 155, Protoss_Pylon = 156, Protoss_Assimilator = 157, Protoss_Observatory = 159, Protoss_Gateway = 160, Protoss_Photon_Cannon = 162, Protoss_Citadel_of_Adun = 163, Protoss_Cybernetics_Core = 164, Protoss_Templar_Archives = 165, Protoss_Forge = 166, Protoss_Stargate = 167, Protoss_Fleet_Beacon = 169, Protoss_Arbiter_Tribunal = 170, Protoss_Robotics_Support_Bay = 171, Protoss_Shield_Battery = 172, Resource_Mineral_Field = 176, Resource_Mineral_Field_Type_2 = 177, Resource_Mineral_Field_Type_3 = 178, Resource_Vespene_Geyser = 188, Spell_Dark_Swarm = 202, Special_Pit_Door = 207, Special_Right_Pit_Door = 208, MAX = 233 }; constexpr UnitType(_enumerated value) : _value(value) { } constexpr operator _enumerated() const { return _enumerated(_value); } constexpr _integral _to_integral() const; constexpr static UnitType _from_integral(_integral value); constexpr static UnitType _from_integral_unchecked(_integral value); constexpr static _optional _from_integral_nothrow(_integral value); const char* _to_string() const; constexpr static UnitType _from_string(const char *name); constexpr static _optional _from_string_nothrow(const char *name); constexpr static UnitType _from_string_nocase(const char *name); constexpr static _optional _from_string_nocase_nothrow(const char *name); constexpr static bool _is_valid(_integral value); constexpr static bool _is_valid(const char *name); constexpr static bool _is_valid_nocase(const char *name); typedef ::better_enums::_Iterable<UnitType> _value_iterable; typedef ::better_enums::_Iterable<const char*> _name_iterable; typedef _value_iterable::iterator _value_iterator; typedef _name_iterable::iterator _name_iterator; constexpr static const std::size_t _size_constant = 118; constexpr static std::size_t _size() { return _size_constant; } constexpr static const char* _name(); constexpr static _value_iterable _values(); static _name_iterable _names(); _integral _value; private: UnitType() : _value(0) { } private: explicit constexpr UnitType(const _integral &value) : _value(value) { } static int initialize(); constexpr static _optional_index _from_value_loop(_integral value, std::size_t index = 0); constexpr static _optional_index _from_string_loop(const char *name, std::size_t index = 0); constexpr static _optional_index _from_string_nocase_loop(const char *name, std::size_t index = 0); friend struct ::better_enums::_initialize_at_program_start<UnitType>; }; namespace better_enums_data_UnitType { static ::better_enums::_initialize_at_program_start<UnitType> _force_initialization; enum _PutNamesInThisScopeAlso { Terran_Marine = 0, Terran_Ghost = 1, Terran_Vulture = 2, Terran_Goliath = 3, Terran_Siege_Tank_Tank_Mode = 5, Terran_SCV = 7, Terran_Wraith = 8, Terran_Science_Vessel = 9, Terran_Dropship = 11, Terran_Battlecruiser = 12, Terran_Vulture_Spider_Mine = 13, Terran_Nuclear_Missile = 14, Terran_Civilian = 15, Terran_Siege_Tank_Siege_Mode = 30, Terran_Firebat = 32, Spell_Scanner_Sweep = 33, Terran_Medic = 34, Zerg_Larva = 35, Zerg_Egg = 36, Zerg_Zergling = 37, Zerg_Hydralisk = 38, Zerg_Ultralisk = 39, Zerg_Broodling = 40, Zerg_Drone = 41, Zerg_Overlord = 42, Zerg_Mutalisk = 43, Zerg_Guardian = 44, Zerg_Queen = 45, Zerg_Defiler = 46, Zerg_Scourge = 47, Zerg_Infested_Terran = 50, Terran_Valkyrie = 58, Zerg_Cocoon = 59, Protoss_Corsair = 60, Protoss_Dark_Templar = 61, Zerg_Devourer = 62, Protoss_Dark_Archon = 63, Protoss_Probe = 64, Protoss_Zealot = 65, Protoss_Dragoon = 66, Protoss_High_Templar = 67, Protoss_Archon = 68, Protoss_Shuttle = 69, Protoss_Scout = 70, Protoss_Arbiter = 71, Protoss_Carrier = 72, Protoss_Interceptor = 73, Protoss_Reaver = 83, Protoss_Observer = 84, Protoss_Scarab = 85, Critter_Rhynadon = 89, Critter_Bengalaas = 90, Critter_Scantid = 93, Critter_Kakaru = 94, Critter_Ragnasaur = 95, Critter_Ursadon = 96, Zerg_Lurker_Egg = 97, Zerg_Lurker = 103, Spell_Disruption_Web = 105, Terran_Command_Center = 106, Terran_Comsat_Station = 107, Terran_Nuclear_Silo = 108, Terran_Supply_Depot = 109, Terran_Refinery = 110, Terran_Barracks = 111, Terran_Academy = 112, Terran_Factory = 113, Terran_Starport = 114, Terran_Control_Tower = 115, Terran_Science_Facility = 116, Terran_Covert_Ops = 117, Terran_Physics_Lab = 118, Terran_Machine_Shop = 120, Terran_Engineering_Bay = 122, Terran_Armory = 123, Terran_Missile_Turret = 124, Terran_Bunker = 125, Zerg_Infested_Command_Center = 130, Zerg_Hatchery = 131, Zerg_Lair = 132, Zerg_Hive = 133, Zerg_Nydus_Canal = 134, Zerg_Hydralisk_Den = 135, Zerg_Defiler_Mound = 136, Zerg_Greater_Spire = 137, Zerg_Queens_Nest = 138, Zerg_Evolution_Chamber = 139, Zerg_Ultralisk_Cavern = 140, Zerg_Spire = 141, Zerg_Spawning_Pool = 142, Zerg_Creep_Colony = 143, Zerg_Spore_Colony = 144, Zerg_Sunken_Colony = 146, Zerg_Extractor = 149, Protoss_Nexus = 154, Protoss_Robotics_Facility = 155, Protoss_Pylon = 156, Protoss_Assimilator = 157, Protoss_Observatory = 159, Protoss_Gateway = 160, Protoss_Photon_Cannon = 162, Protoss_Citadel_of_Adun = 163, Protoss_Cybernetics_Core = 164, Protoss_Templar_Archives = 165, Protoss_Forge = 166, Protoss_Stargate = 167, Protoss_Fleet_Beacon = 169, Protoss_Arbiter_Tribunal = 170, Protoss_Robotics_Support_Bay = 171, Protoss_Shield_Battery = 172, Resource_Mineral_Field = 176, Resource_Mineral_Field_Type_2 = 177, Resource_Mineral_Field_Type_3 = 178, Resource_Vespene_Geyser = 188, Spell_Dark_Swarm = 202, Special_Pit_Door = 207, Special_Right_Pit_Door = 208, MAX = 233 }; constexpr const UnitType _value_array[] = { ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Marine = 0), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Ghost = 1), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Vulture = 2), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Goliath = 3), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Siege_Tank_Tank_Mode = 5), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_SCV = 7), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Wraith = 8), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Science_Vessel = 9), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Dropship = 11), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Battlecruiser = 12), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Vulture_Spider_Mine = 13), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Nuclear_Missile = 14), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Civilian = 15), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Siege_Tank_Siege_Mode = 30), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Firebat = 32), ((::better_enums::_eat_assign<UnitType>)UnitType::Spell_Scanner_Sweep = 33), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Medic = 34), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Larva = 35), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Egg = 36), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Zergling = 37), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Hydralisk = 38), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Ultralisk = 39), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Broodling = 40), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Drone = 41), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Overlord = 42), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Mutalisk = 43), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Guardian = 44), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Queen = 45), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Defiler = 46), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Scourge = 47), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Infested_Terran = 50), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Valkyrie = 58), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Cocoon = 59), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Corsair = 60), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Dark_Templar = 61), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Devourer = 62), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Dark_Archon = 63), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Probe = 64), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Zealot = 65), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Dragoon = 66), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_High_Templar = 67), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Archon = 68), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Shuttle = 69), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Scout = 70), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Arbiter = 71), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Carrier = 72), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Interceptor = 73), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Reaver = 83), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Observer = 84), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Scarab = 85), ((::better_enums::_eat_assign<UnitType>)UnitType::Critter_Rhynadon = 89), ((::better_enums::_eat_assign<UnitType>)UnitType::Critter_Bengalaas = 90), ((::better_enums::_eat_assign<UnitType>)UnitType::Critter_Scantid = 93), ((::better_enums::_eat_assign<UnitType>)UnitType::Critter_Kakaru = 94), ((::better_enums::_eat_assign<UnitType>)UnitType::Critter_Ragnasaur = 95), ((::better_enums::_eat_assign<UnitType>)UnitType::Critter_Ursadon = 96), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Lurker_Egg = 97), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Lurker = 103), ((::better_enums::_eat_assign<UnitType>)UnitType::Spell_Disruption_Web = 105), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Command_Center = 106), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Comsat_Station = 107), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Nuclear_Silo = 108), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Supply_Depot = 109), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Refinery = 110), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Barracks = 111), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Academy = 112), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Factory = 113), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Starport = 114), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Control_Tower = 115), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Science_Facility = 116), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Covert_Ops = 117), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Physics_Lab = 118), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Machine_Shop = 120), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Engineering_Bay = 122), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Armory = 123), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Missile_Turret = 124), ((::better_enums::_eat_assign<UnitType>)UnitType::Terran_Bunker = 125), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Infested_Command_Center = 130), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Hatchery = 131), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Lair = 132), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Hive = 133), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Nydus_Canal = 134), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Hydralisk_Den = 135), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Defiler_Mound = 136), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Greater_Spire = 137), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Queens_Nest = 138), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Evolution_Chamber = 139), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Ultralisk_Cavern = 140), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Spire = 141), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Spawning_Pool = 142), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Creep_Colony = 143), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Spore_Colony = 144), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Sunken_Colony = 146), ((::better_enums::_eat_assign<UnitType>)UnitType::Zerg_Extractor = 149), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Nexus = 154), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Robotics_Facility = 155), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Pylon = 156), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Assimilator = 157), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Observatory = 159), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Gateway = 160), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Photon_Cannon = 162), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Citadel_of_Adun = 163), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Cybernetics_Core = 164), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Templar_Archives = 165), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Forge = 166), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Stargate = 167), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Fleet_Beacon = 169), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Arbiter_Tribunal = 170), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Robotics_Support_Bay = 171), ((::better_enums::_eat_assign<UnitType>)UnitType::Protoss_Shield_Battery = 172), ((::better_enums::_eat_assign<UnitType>)UnitType::Resource_Mineral_Field = 176), ((::better_enums::_eat_assign<UnitType>)UnitType::Resource_Mineral_Field_Type_2 = 177), ((::better_enums::_eat_assign<UnitType>)UnitType::Resource_Mineral_Field_Type_3 = 178), ((::better_enums::_eat_assign<UnitType>)UnitType::Resource_Vespene_Geyser = 188), ((::better_enums::_eat_assign<UnitType>)UnitType::Spell_Dark_Swarm = 202), ((::better_enums::_eat_assign<UnitType>)UnitType::Special_Pit_Door = 207), ((::better_enums::_eat_assign<UnitType>)UnitType::Special_Right_Pit_Door = 208), ((::better_enums::_eat_assign<UnitType>)UnitType::MAX = 233), }; constexpr const char *_the_raw_names[] = { "Terran_Marine = 0", "Terran_Ghost = 1", "Terran_Vulture = 2", "Terran_Goliath = 3", "Terran_Siege_Tank_Tank_Mode = 5", "Terran_SCV = 7", "Terran_Wraith = 8", "Terran_Science_Vessel = 9", "Terran_Dropship = 11", "Terran_Battlecruiser = 12", "Terran_Vulture_Spider_Mine = 13", "Terran_Nuclear_Missile = 14", "Terran_Civilian = 15", "Terran_Siege_Tank_Siege_Mode = 30", "Terran_Firebat = 32", "Spell_Scanner_Sweep = 33", "Terran_Medic = 34", "Zerg_Larva = 35", "Zerg_Egg = 36", "Zerg_Zergling = 37", "Zerg_Hydralisk = 38", "Zerg_Ultralisk = 39", "Zerg_Broodling = 40", "Zerg_Drone = 41", "Zerg_Overlord = 42", "Zerg_Mutalisk = 43", "Zerg_Guardian = 44", "Zerg_Queen = 45", "Zerg_Defiler = 46", "Zerg_Scourge = 47", "Zerg_Infested_Terran = 50", "Terran_Valkyrie = 58", "Zerg_Cocoon = 59", "Protoss_Corsair = 60", "Protoss_Dark_Templar = 61", "Zerg_Devourer = 62", "Protoss_Dark_Archon = 63", "Protoss_Probe = 64", "Protoss_Zealot = 65", "Protoss_Dragoon = 66", "Protoss_High_Templar = 67", "Protoss_Archon = 68", "Protoss_Shuttle = 69", "Protoss_Scout = 70", "Protoss_Arbiter = 71", "Protoss_Carrier = 72", "Protoss_Interceptor = 73", "Protoss_Reaver = 83", "Protoss_Observer = 84", "Protoss_Scarab = 85", "Critter_Rhynadon = 89", "Critter_Bengalaas = 90", "Critter_Scantid = 93", "Critter_Kakaru = 94", "Critter_Ragnasaur = 95", "Critter_Ursadon = 96", "Zerg_Lurker_Egg = 97", "Zerg_Lurker = 103", "Spell_Disruption_Web = 105", "Terran_Command_Center = 106", "Terran_Comsat_Station = 107", "Terran_Nuclear_Silo = 108", "Terran_Supply_Depot = 109", "Terran_Refinery = 110", "Terran_Barracks = 111", "Terran_Academy = 112", "Terran_Factory = 113", "Terran_Starport = 114", "Terran_Control_Tower = 115", "Terran_Science_Facility = 116", "Terran_Covert_Ops = 117", "Terran_Physics_Lab = 118", "Terran_Machine_Shop = 120", "Terran_Engineering_Bay = 122", "Terran_Armory = 123", "Terran_Missile_Turret = 124", "Terran_Bunker = 125", "Zerg_Infested_Command_Center = 130", "Zerg_Hatchery = 131", "Zerg_Lair = 132", "Zerg_Hive = 133", "Zerg_Nydus_Canal = 134", "Zerg_Hydralisk_Den = 135", "Zerg_Defiler_Mound = 136", "Zerg_Greater_Spire = 137", "Zerg_Queens_Nest = 138", "Zerg_Evolution_Chamber = 139", "Zerg_Ultralisk_Cavern = 140", "Zerg_Spire = 141", "Zerg_Spawning_Pool = 142", "Zerg_Creep_Colony = 143", "Zerg_Spore_Colony = 144", "Zerg_Sunken_Colony = 146", "Zerg_Extractor = 149", "Protoss_Nexus = 154", "Protoss_Robotics_Facility = 155", "Protoss_Pylon = 156", "Protoss_Assimilator = 157", "Protoss_Observatory = 159", "Protoss_Gateway = 160", "Protoss_Photon_Cannon = 162", "Protoss_Citadel_of_Adun = 163", "Protoss_Cybernetics_Core = 164", "Protoss_Templar_Archives = 165", "Protoss_Forge = 166", "Protoss_Stargate = 167", "Protoss_Fleet_Beacon = 169", "Protoss_Arbiter_Tribunal = 170", "Protoss_Robotics_Support_Bay = 171", "Protoss_Shield_Battery = 172", "Resource_Mineral_Field = 176", "Resource_Mineral_Field_Type_2 = 177", "Resource_Mineral_Field_Type_3 = 178", "Resource_Vespene_Geyser = 188", "Spell_Dark_Swarm = 202", "Special_Pit_Door = 207", "Special_Right_Pit_Door = 208", "MAX = 233", }; constexpr const char * const * _raw_names() { return _the_raw_names; } inline char* _name_storage() { static char storage[] = "Terran_Marine = 0" "," "Terran_Ghost = 1" "," "Terran_Vulture = 2" "," "Terran_Goliath = 3" "," "Terran_Siege_Tank_Tank_Mode = 5" "," "Terran_SCV = 7" "," "Terran_Wraith = 8" "," "Terran_Science_Vessel = 9" "," "Terran_Dropship = 11" "," "Terran_Battlecruiser = 12" "," "Terran_Vulture_Spider_Mine = 13" "," "Terran_Nuclear_Missile = 14" "," "Terran_Civilian = 15" "," "Terran_Siege_Tank_Siege_Mode = 30" "," "Terran_Firebat = 32" "," "Spell_Scanner_Sweep = 33" "," "Terran_Medic = 34" "," "Zerg_Larva = 35" "," "Zerg_Egg = 36" "," "Zerg_Zergling = 37" "," "Zerg_Hydralisk = 38" "," "Zerg_Ultralisk = 39" "," "Zerg_Broodling = 40" "," "Zerg_Drone = 41" "," "Zerg_Overlord = 42" "," "Zerg_Mutalisk = 43" "," "Zerg_Guardian = 44" "," "Zerg_Queen = 45" "," "Zerg_Defiler = 46" "," "Zerg_Scourge = 47" "," "Zerg_Infested_Terran = 50" "," "Terran_Valkyrie = 58" "," "Zerg_Cocoon = 59" "," "Protoss_Corsair = 60" "," "Protoss_Dark_Templar = 61" "," "Zerg_Devourer = 62" "," "Protoss_Dark_Archon = 63" "," "Protoss_Probe = 64" "," "Protoss_Zealot = 65" "," "Protoss_Dragoon = 66" "," "Protoss_High_Templar = 67" "," "Protoss_Archon = 68" "," "Protoss_Shuttle = 69" "," "Protoss_Scout = 70" "," "Protoss_Arbiter = 71" "," "Protoss_Carrier = 72" "," "Protoss_Interceptor = 73" "," "Protoss_Reaver = 83" "," "Protoss_Observer = 84" "," "Protoss_Scarab = 85" "," "Critter_Rhynadon = 89" "," "Critter_Bengalaas = 90" "," "Critter_Scantid = 93" "," "Critter_Kakaru = 94" "," "Critter_Ragnasaur = 95" "," "Critter_Ursadon = 96" "," "Zerg_Lurker_Egg = 97" "," "Zerg_Lurker = 103" "," "Spell_Disruption_Web = 105" "," "Terran_Command_Center = 106" "," "Terran_Comsat_Station = 107" "," "Terran_Nuclear_Silo = 108" "," "Terran_Supply_Depot = 109" "," "Terran_Refinery = 110" "," "Terran_Barracks = 111" "," "Terran_Academy = 112" "," "Terran_Factory = 113" "," "Terran_Starport = 114" "," "Terran_Control_Tower = 115" "," "Terran_Science_Facility = 116" "," "Terran_Covert_Ops = 117" "," "Terran_Physics_Lab = 118" "," "Terran_Machine_Shop = 120" "," "Terran_Engineering_Bay = 122" "," "Terran_Armory = 123" "," "Terran_Missile_Turret = 124" "," "Terran_Bunker = 125" "," "Zerg_Infested_Command_Center = 130" "," "Zerg_Hatchery = 131" "," "Zerg_Lair = 132" "," "Zerg_Hive = 133" "," "Zerg_Nydus_Canal = 134" "," "Zerg_Hydralisk_Den = 135" "," "Zerg_Defiler_Mound = 136" "," "Zerg_Greater_Spire = 137" "," "Zerg_Queens_Nest = 138" "," "Zerg_Evolution_Chamber = 139" "," "Zerg_Ultralisk_Cavern = 140" "," "Zerg_Spire = 141" "," "Zerg_Spawning_Pool = 142" "," "Zerg_Creep_Colony = 143" "," "Zerg_Spore_Colony = 144" "," "Zerg_Sunken_Colony = 146" "," "Zerg_Extractor = 149" "," "Protoss_Nexus = 154" "," "Protoss_Robotics_Facility = 155" "," "Protoss_Pylon = 156" "," "Protoss_Assimilator = 157" "," "Protoss_Observatory = 159" "," "Protoss_Gateway = 160" "," "Protoss_Photon_Cannon = 162" "," "Protoss_Citadel_of_Adun = 163" "," "Protoss_Cybernetics_Core = 164" "," "Protoss_Templar_Archives = 165" "," "Protoss_Forge = 166" "," "Protoss_Stargate = 167" "," "Protoss_Fleet_Beacon = 169" "," "Protoss_Arbiter_Tribunal = 170" "," "Protoss_Robotics_Support_Bay = 171" "," "Protoss_Shield_Battery = 172" "," "Resource_Mineral_Field = 176" "," "Resource_Mineral_Field_Type_2 = 177" "," "Resource_Mineral_Field_Type_3 = 178" "," "Resource_Vespene_Geyser = 188" "," "Spell_Dark_Swarm = 202" "," "Special_Pit_Door = 207" "," "Special_Right_Pit_Door = 208" "," "MAX = 233" ","; return storage; } inline const char** _name_array() { static const char *value[UnitType::_size_constant]; return value; } inline bool& _initialized() { static bool value = false; return value; } }  constexpr inline const UnitType operator +(UnitType::_enumerated enumerated) { return static_cast<UnitType>(enumerated); } constexpr inline UnitType::_optional_index UnitType::_from_value_loop(UnitType::_integral value, std::size_t index) { return index == _size() ? _optional_index() : better_enums_data_UnitType::_value_array[index]._value == value ? _optional_index(index) : _from_value_loop(value, index + 1); } constexpr inline UnitType::_optional_index UnitType::_from_string_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match(better_enums_data_UnitType::_raw_names()[index], name) ? _optional_index(index) : _from_string_loop(name, index + 1); } constexpr inline UnitType::_optional_index UnitType::_from_string_nocase_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match_nocase(better_enums_data_UnitType::_raw_names()[index], name) ? _optional_index(index) : _from_string_nocase_loop(name, index + 1); } constexpr inline UnitType::_integral UnitType::_to_integral() const { return _integral(_value); } constexpr inline UnitType UnitType::_from_integral_unchecked(_integral value) { return static_cast<_enumerated>(value); } constexpr inline UnitType::_optional UnitType::_from_integral_nothrow(_integral value) { return ::better_enums::_map_index<UnitType>(better_enums_data_UnitType::_value_array, _from_value_loop(value)); } constexpr inline UnitType UnitType::_from_integral(_integral value) { return ::better_enums::_or_throw(_from_integral_nothrow(value), "UnitType" "::_from_integral: invalid argument"); } inline const char* UnitType::_to_string() const { return ::better_enums::_or_null(::better_enums::_map_index<const char*>(better_enums_data_UnitType::_name_array(), _from_value_loop(::better_enums::continue_with(initialize(), _value)))); } constexpr inline UnitType::_optional UnitType::_from_string_nothrow(const char *name) { return ::better_enums::_map_index<UnitType>(better_enums_data_UnitType::_value_array, _from_string_loop(name)); } constexpr inline UnitType UnitType::_from_string(const char *name) { return ::better_enums::_or_throw(_from_string_nothrow(name), "UnitType" "::_from_string: invalid argument"); } constexpr inline UnitType::_optional UnitType::_from_string_nocase_nothrow(const char *name) { return ::better_enums::_map_index<UnitType>(better_enums_data_UnitType::_value_array, _from_string_nocase_loop(name)); } constexpr inline UnitType UnitType::_from_string_nocase(const char *name) { return ::better_enums::_or_throw(_from_string_nocase_nothrow(name), "UnitType" "::_from_string_nocase: invalid argument"); } constexpr inline bool UnitType::_is_valid(_integral value) { return _from_value_loop(value); } constexpr inline bool UnitType::_is_valid(const char *name) { return _from_string_loop(name); } constexpr inline bool UnitType::_is_valid_nocase(const char *name) { return _from_string_nocase_loop(name); } constexpr inline const char* UnitType::_name() { return "UnitType"; } constexpr inline UnitType::_value_iterable UnitType::_values() { return _value_iterable(better_enums_data_UnitType::_value_array, _size()); } inline UnitType::_name_iterable UnitType::_names() { return _name_iterable(better_enums_data_UnitType::_name_array(), ::better_enums::continue_with(initialize(), _size())); } inline int UnitType::initialize() { if (better_enums_data_UnitType::_initialized()) return 0; ::better_enums::_trim_names(better_enums_data_UnitType::_raw_names(), better_enums_data_UnitType::_name_array(), better_enums_data_UnitType::_name_storage(), _size()); better_enums_data_UnitType::_initialized() = true; return 0; }  constexpr inline bool operator ==(const UnitType &a, const UnitType &b) { return a._to_integral() == b._to_integral(); }  constexpr inline bool operator !=(const UnitType &a, const UnitType &b) { return a._to_integral() != b._to_integral(); }  constexpr inline bool operator <(const UnitType &a, const UnitType &b) { return a._to_integral() < b._to_integral(); }  constexpr inline bool operator <=(const UnitType &a, const UnitType &b) { return a._to_integral() <= b._to_integral(); }  constexpr inline bool operator >(const UnitType &a, const UnitType &b) { return a._to_integral() > b._to_integral(); }  constexpr inline bool operator >=(const UnitType &a, const UnitType &b) { return a._to_integral() >= b._to_integral(); } template <typename Char, typename Traits> std::basic_ostream<Char, Traits>& operator <<(std::basic_ostream<Char, Traits>& stream, const UnitType &value) { return stream << value._to_string(); } template <typename Char, typename Traits> std::basic_istream<Char, Traits>& operator >>(std::basic_istream<Char, Traits>& stream, UnitType &value) { std::basic_string<Char, Traits> buffer; stream >> buffer; ::better_enums::optional<UnitType> converted = UnitType::_from_string_nothrow(buffer.c_str()); if (converted) value = *converted; else stream.setstate(std::basic_istream<Char, Traits>::failbit); return stream; }
#endif // TC_EXPAND_LARGE_ENUMS

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

#ifndef TC_EXPAND_LARGE_ENUMS
// Be sure to update the expanded version below when changing this
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
#else // TC_EXPAND_LARGE_ENUMS
namespace better_enums_data_WeaponType {} class WeaponType { private: typedef ::better_enums::optional<WeaponType> _optional; typedef ::better_enums::optional<std::size_t> _optional_index; public: typedef int _integral; enum _enumerated : int { Gauss_Rifle = 0, Gauss_Rifle_Jim_Raynor = 1, C_10_Canister_Rifle = 2, C_10_Canister_Rifle_Sarah_Kerrigan = 3, Fragmentation_Grenade = 4, Fragmentation_Grenade_Jim_Raynor = 5, Spider_Mines = 6, Twin_Autocannons = 7, Hellfire_Missile_Pack = 8, Twin_Autocannons_Alan_Schezar = 9, Hellfire_Missile_Pack_Alan_Schezar = 10, Arclite_Cannon = 11, Arclite_Cannon_Edmund_Duke = 12, Fusion_Cutter = 13, Gemini_Missiles = 15, Burst_Lasers = 16, Gemini_Missiles_Tom_Kazansky = 17, Burst_Lasers_Tom_Kazansky = 18, ATS_Laser_Battery = 19, ATA_Laser_Battery = 20, ATS_Laser_Battery_Hero = 21, ATA_Laser_Battery_Hero = 22, ATS_Laser_Battery_Hyperion = 23, ATA_Laser_Battery_Hyperion = 24, Flame_Thrower = 25, Flame_Thrower_Gui_Montag = 26, Arclite_Shock_Cannon = 27, Arclite_Shock_Cannon_Edmund_Duke = 28, Longbolt_Missile = 29, Yamato_Gun = 30, Nuclear_Strike = 31, Lockdown = 32, EMP_Shockwave = 33, Irradiate = 34, Claws = 35, Claws_Devouring_One = 36, Claws_Infested_Kerrigan = 37, Needle_Spines = 38, Needle_Spines_Hunter_Killer = 39, Kaiser_Blades = 40, Kaiser_Blades_Torrasque = 41, Toxic_Spores = 42, Spines = 43, Acid_Spore = 46, Acid_Spore_Kukulza = 47, Glave_Wurm = 48, Glave_Wurm_Kukulza = 49, Seeker_Spores = 52, Subterranean_Tentacle = 53, Suicide_Infested_Terran = 54, Suicide_Scourge = 55, Parasite = 56, Spawn_Broodlings = 57, Ensnare = 58, Dark_Swarm = 59, Plague = 60, Consume = 61, Particle_Beam = 62, Psi_Blades = 64, Psi_Blades_Fenix = 65, Phase_Disruptor = 66, Phase_Disruptor_Fenix = 67, Psi_Assault = 69, Psionic_Shockwave = 70, Psionic_Shockwave_TZ_Archon = 71, Dual_Photon_Blasters = 73, Anti_Matter_Missiles = 74, Dual_Photon_Blasters_Mojo = 75, Anti_Matter_Missiles_Mojo = 76, Phase_Disruptor_Cannon = 77, Phase_Disruptor_Cannon_Danimoth = 78, Pulse_Cannon = 79, STS_Photon_Cannon = 80, STA_Photon_Cannon = 81, Scarab = 82, Stasis_Field = 83, Psionic_Storm = 84, Warp_Blades_Zeratul = 85, Warp_Blades_Hero = 86, Platform_Laser_Battery = 92, Independant_Laser_Battery = 93, Twin_Autocannons_Floor_Trap = 96, Hellfire_Missile_Pack_Wall_Trap = 97, Flame_Thrower_Wall_Trap = 98, Hellfire_Missile_Pack_Floor_Trap = 99, Neutron_Flare = 100, Disruption_Web = 101, Restoration = 102, Halo_Rockets = 103, Corrosive_Acid = 104, Mind_Control = 105, Feedback = 106, Optical_Flare = 107, Maelstrom = 108, Subterranean_Spines = 109, Warp_Blades = 111, C_10_Canister_Rifle_Samir_Duran = 112, C_10_Canister_Rifle_Infested_Duran = 113, Dual_Photon_Blasters_Artanis = 114, Anti_Matter_Missiles_Artanis = 115, C_10_Canister_Rifle_Alexei_Stukov = 116, None = 130, Unknown, MAX }; constexpr WeaponType(_enumerated value) : _value(value) { } constexpr operator _enumerated() const { return _enumerated(_value); } constexpr _integral _to_integral() const; constexpr static WeaponType _from_integral(_integral value); constexpr static WeaponType _from_integral_unchecked(_integral value); constexpr static _optional _from_integral_nothrow(_integral value); const char* _to_string() const; constexpr static WeaponType _from_string(const char *name); constexpr static _optional _from_string_nothrow(const char *name); constexpr static WeaponType _from_string_nocase(const char *name); constexpr static _optional _from_string_nocase_nothrow(const char *name); constexpr static bool _is_valid(_integral value); constexpr static bool _is_valid(const char *name); constexpr static bool _is_valid_nocase(const char *name); typedef ::better_enums::_Iterable<WeaponType> _value_iterable; typedef ::better_enums::_Iterable<const char*> _name_iterable; typedef _value_iterable::iterator _value_iterator; typedef _name_iterable::iterator _name_iterator; constexpr static const std::size_t _size_constant = 104; constexpr static std::size_t _size() { return _size_constant; } constexpr static const char* _name(); constexpr static _value_iterable _values(); static _name_iterable _names(); _integral _value; private: WeaponType() : _value(0) { } private: explicit constexpr WeaponType(const _integral &value) : _value(value) { } static int initialize(); constexpr static _optional_index _from_value_loop(_integral value, std::size_t index = 0); constexpr static _optional_index _from_string_loop(const char *name, std::size_t index = 0); constexpr static _optional_index _from_string_nocase_loop(const char *name, std::size_t index = 0); friend struct ::better_enums::_initialize_at_program_start<WeaponType>; }; namespace better_enums_data_WeaponType { static ::better_enums::_initialize_at_program_start<WeaponType> _force_initialization; enum _PutNamesInThisScopeAlso { Gauss_Rifle = 0, Gauss_Rifle_Jim_Raynor = 1, C_10_Canister_Rifle = 2, C_10_Canister_Rifle_Sarah_Kerrigan = 3, Fragmentation_Grenade = 4, Fragmentation_Grenade_Jim_Raynor = 5, Spider_Mines = 6, Twin_Autocannons = 7, Hellfire_Missile_Pack = 8, Twin_Autocannons_Alan_Schezar = 9, Hellfire_Missile_Pack_Alan_Schezar = 10, Arclite_Cannon = 11, Arclite_Cannon_Edmund_Duke = 12, Fusion_Cutter = 13, Gemini_Missiles = 15, Burst_Lasers = 16, Gemini_Missiles_Tom_Kazansky = 17, Burst_Lasers_Tom_Kazansky = 18, ATS_Laser_Battery = 19, ATA_Laser_Battery = 20, ATS_Laser_Battery_Hero = 21, ATA_Laser_Battery_Hero = 22, ATS_Laser_Battery_Hyperion = 23, ATA_Laser_Battery_Hyperion = 24, Flame_Thrower = 25, Flame_Thrower_Gui_Montag = 26, Arclite_Shock_Cannon = 27, Arclite_Shock_Cannon_Edmund_Duke = 28, Longbolt_Missile = 29, Yamato_Gun = 30, Nuclear_Strike = 31, Lockdown = 32, EMP_Shockwave = 33, Irradiate = 34, Claws = 35, Claws_Devouring_One = 36, Claws_Infested_Kerrigan = 37, Needle_Spines = 38, Needle_Spines_Hunter_Killer = 39, Kaiser_Blades = 40, Kaiser_Blades_Torrasque = 41, Toxic_Spores = 42, Spines = 43, Acid_Spore = 46, Acid_Spore_Kukulza = 47, Glave_Wurm = 48, Glave_Wurm_Kukulza = 49, Seeker_Spores = 52, Subterranean_Tentacle = 53, Suicide_Infested_Terran = 54, Suicide_Scourge = 55, Parasite = 56, Spawn_Broodlings = 57, Ensnare = 58, Dark_Swarm = 59, Plague = 60, Consume = 61, Particle_Beam = 62, Psi_Blades = 64, Psi_Blades_Fenix = 65, Phase_Disruptor = 66, Phase_Disruptor_Fenix = 67, Psi_Assault = 69, Psionic_Shockwave = 70, Psionic_Shockwave_TZ_Archon = 71, Dual_Photon_Blasters = 73, Anti_Matter_Missiles = 74, Dual_Photon_Blasters_Mojo = 75, Anti_Matter_Missiles_Mojo = 76, Phase_Disruptor_Cannon = 77, Phase_Disruptor_Cannon_Danimoth = 78, Pulse_Cannon = 79, STS_Photon_Cannon = 80, STA_Photon_Cannon = 81, Scarab = 82, Stasis_Field = 83, Psionic_Storm = 84, Warp_Blades_Zeratul = 85, Warp_Blades_Hero = 86, Platform_Laser_Battery = 92, Independant_Laser_Battery = 93, Twin_Autocannons_Floor_Trap = 96, Hellfire_Missile_Pack_Wall_Trap = 97, Flame_Thrower_Wall_Trap = 98, Hellfire_Missile_Pack_Floor_Trap = 99, Neutron_Flare = 100, Disruption_Web = 101, Restoration = 102, Halo_Rockets = 103, Corrosive_Acid = 104, Mind_Control = 105, Feedback = 106, Optical_Flare = 107, Maelstrom = 108, Subterranean_Spines = 109, Warp_Blades = 111, C_10_Canister_Rifle_Samir_Duran = 112, C_10_Canister_Rifle_Infested_Duran = 113, Dual_Photon_Blasters_Artanis = 114, Anti_Matter_Missiles_Artanis = 115, C_10_Canister_Rifle_Alexei_Stukov = 116, None = 130, Unknown, MAX }; constexpr const WeaponType _value_array[] = { ((::better_enums::_eat_assign<WeaponType>)WeaponType::Gauss_Rifle = 0), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Gauss_Rifle_Jim_Raynor = 1), ((::better_enums::_eat_assign<WeaponType>)WeaponType::C_10_Canister_Rifle = 2), ((::better_enums::_eat_assign<WeaponType>)WeaponType::C_10_Canister_Rifle_Sarah_Kerrigan = 3), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Fragmentation_Grenade = 4), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Fragmentation_Grenade_Jim_Raynor = 5), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Spider_Mines = 6), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Twin_Autocannons = 7), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Hellfire_Missile_Pack = 8), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Twin_Autocannons_Alan_Schezar = 9), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Hellfire_Missile_Pack_Alan_Schezar = 10), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Arclite_Cannon = 11), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Arclite_Cannon_Edmund_Duke = 12), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Fusion_Cutter = 13), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Gemini_Missiles = 15), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Burst_Lasers = 16), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Gemini_Missiles_Tom_Kazansky = 17), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Burst_Lasers_Tom_Kazansky = 18), ((::better_enums::_eat_assign<WeaponType>)WeaponType::ATS_Laser_Battery = 19), ((::better_enums::_eat_assign<WeaponType>)WeaponType::ATA_Laser_Battery = 20), ((::better_enums::_eat_assign<WeaponType>)WeaponType::ATS_Laser_Battery_Hero = 21), ((::better_enums::_eat_assign<WeaponType>)WeaponType::ATA_Laser_Battery_Hero = 22), ((::better_enums::_eat_assign<WeaponType>)WeaponType::ATS_Laser_Battery_Hyperion = 23), ((::better_enums::_eat_assign<WeaponType>)WeaponType::ATA_Laser_Battery_Hyperion = 24), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Flame_Thrower = 25), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Flame_Thrower_Gui_Montag = 26), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Arclite_Shock_Cannon = 27), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Arclite_Shock_Cannon_Edmund_Duke = 28), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Longbolt_Missile = 29), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Yamato_Gun = 30), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Nuclear_Strike = 31), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Lockdown = 32), ((::better_enums::_eat_assign<WeaponType>)WeaponType::EMP_Shockwave = 33), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Irradiate = 34), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Claws = 35), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Claws_Devouring_One = 36), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Claws_Infested_Kerrigan = 37), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Needle_Spines = 38), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Needle_Spines_Hunter_Killer = 39), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Kaiser_Blades = 40), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Kaiser_Blades_Torrasque = 41), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Toxic_Spores = 42), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Spines = 43), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Acid_Spore = 46), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Acid_Spore_Kukulza = 47), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Glave_Wurm = 48), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Glave_Wurm_Kukulza = 49), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Seeker_Spores = 52), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Subterranean_Tentacle = 53), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Suicide_Infested_Terran = 54), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Suicide_Scourge = 55), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Parasite = 56), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Spawn_Broodlings = 57), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Ensnare = 58), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Dark_Swarm = 59), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Plague = 60), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Consume = 61), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Particle_Beam = 62), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Psi_Blades = 64), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Psi_Blades_Fenix = 65), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Phase_Disruptor = 66), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Phase_Disruptor_Fenix = 67), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Psi_Assault = 69), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Psionic_Shockwave = 70), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Psionic_Shockwave_TZ_Archon = 71), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Dual_Photon_Blasters = 73), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Anti_Matter_Missiles = 74), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Dual_Photon_Blasters_Mojo = 75), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Anti_Matter_Missiles_Mojo = 76), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Phase_Disruptor_Cannon = 77), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Phase_Disruptor_Cannon_Danimoth = 78), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Pulse_Cannon = 79), ((::better_enums::_eat_assign<WeaponType>)WeaponType::STS_Photon_Cannon = 80), ((::better_enums::_eat_assign<WeaponType>)WeaponType::STA_Photon_Cannon = 81), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Scarab = 82), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Stasis_Field = 83), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Psionic_Storm = 84), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Warp_Blades_Zeratul = 85), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Warp_Blades_Hero = 86), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Platform_Laser_Battery = 92), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Independant_Laser_Battery = 93), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Twin_Autocannons_Floor_Trap = 96), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Hellfire_Missile_Pack_Wall_Trap = 97), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Flame_Thrower_Wall_Trap = 98), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Hellfire_Missile_Pack_Floor_Trap = 99), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Neutron_Flare = 100), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Disruption_Web = 101), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Restoration = 102), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Halo_Rockets = 103), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Corrosive_Acid = 104), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Mind_Control = 105), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Feedback = 106), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Optical_Flare = 107), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Maelstrom = 108), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Subterranean_Spines = 109), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Warp_Blades = 111), ((::better_enums::_eat_assign<WeaponType>)WeaponType::C_10_Canister_Rifle_Samir_Duran = 112), ((::better_enums::_eat_assign<WeaponType>)WeaponType::C_10_Canister_Rifle_Infested_Duran = 113), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Dual_Photon_Blasters_Artanis = 114), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Anti_Matter_Missiles_Artanis = 115), ((::better_enums::_eat_assign<WeaponType>)WeaponType::C_10_Canister_Rifle_Alexei_Stukov = 116), ((::better_enums::_eat_assign<WeaponType>)WeaponType::None = 130), ((::better_enums::_eat_assign<WeaponType>)WeaponType::Unknown), ((::better_enums::_eat_assign<WeaponType>)WeaponType::MAX), }; constexpr const char *_the_raw_names[] = { "Gauss_Rifle = 0", "Gauss_Rifle_Jim_Raynor = 1", "C_10_Canister_Rifle = 2", "C_10_Canister_Rifle_Sarah_Kerrigan = 3", "Fragmentation_Grenade = 4", "Fragmentation_Grenade_Jim_Raynor = 5", "Spider_Mines = 6", "Twin_Autocannons = 7", "Hellfire_Missile_Pack = 8", "Twin_Autocannons_Alan_Schezar = 9", "Hellfire_Missile_Pack_Alan_Schezar = 10", "Arclite_Cannon = 11", "Arclite_Cannon_Edmund_Duke = 12", "Fusion_Cutter = 13", "Gemini_Missiles = 15", "Burst_Lasers = 16", "Gemini_Missiles_Tom_Kazansky = 17", "Burst_Lasers_Tom_Kazansky = 18", "ATS_Laser_Battery = 19", "ATA_Laser_Battery = 20", "ATS_Laser_Battery_Hero = 21", "ATA_Laser_Battery_Hero = 22", "ATS_Laser_Battery_Hyperion = 23", "ATA_Laser_Battery_Hyperion = 24", "Flame_Thrower = 25", "Flame_Thrower_Gui_Montag = 26", "Arclite_Shock_Cannon = 27", "Arclite_Shock_Cannon_Edmund_Duke = 28", "Longbolt_Missile = 29", "Yamato_Gun = 30", "Nuclear_Strike = 31", "Lockdown = 32", "EMP_Shockwave = 33", "Irradiate = 34", "Claws = 35", "Claws_Devouring_One = 36", "Claws_Infested_Kerrigan = 37", "Needle_Spines = 38", "Needle_Spines_Hunter_Killer = 39", "Kaiser_Blades = 40", "Kaiser_Blades_Torrasque = 41", "Toxic_Spores = 42", "Spines = 43", "Acid_Spore = 46", "Acid_Spore_Kukulza = 47", "Glave_Wurm = 48", "Glave_Wurm_Kukulza = 49", "Seeker_Spores = 52", "Subterranean_Tentacle = 53", "Suicide_Infested_Terran = 54", "Suicide_Scourge = 55", "Parasite = 56", "Spawn_Broodlings = 57", "Ensnare = 58", "Dark_Swarm = 59", "Plague = 60", "Consume = 61", "Particle_Beam = 62", "Psi_Blades = 64", "Psi_Blades_Fenix = 65", "Phase_Disruptor = 66", "Phase_Disruptor_Fenix = 67", "Psi_Assault = 69", "Psionic_Shockwave = 70", "Psionic_Shockwave_TZ_Archon = 71", "Dual_Photon_Blasters = 73", "Anti_Matter_Missiles = 74", "Dual_Photon_Blasters_Mojo = 75", "Anti_Matter_Missiles_Mojo = 76", "Phase_Disruptor_Cannon = 77", "Phase_Disruptor_Cannon_Danimoth = 78", "Pulse_Cannon = 79", "STS_Photon_Cannon = 80", "STA_Photon_Cannon = 81", "Scarab = 82", "Stasis_Field = 83", "Psionic_Storm = 84", "Warp_Blades_Zeratul = 85", "Warp_Blades_Hero = 86", "Platform_Laser_Battery = 92", "Independant_Laser_Battery = 93", "Twin_Autocannons_Floor_Trap = 96", "Hellfire_Missile_Pack_Wall_Trap = 97", "Flame_Thrower_Wall_Trap = 98", "Hellfire_Missile_Pack_Floor_Trap = 99", "Neutron_Flare = 100", "Disruption_Web = 101", "Restoration = 102", "Halo_Rockets = 103", "Corrosive_Acid = 104", "Mind_Control = 105", "Feedback = 106", "Optical_Flare = 107", "Maelstrom = 108", "Subterranean_Spines = 109", "Warp_Blades = 111", "C_10_Canister_Rifle_Samir_Duran = 112", "C_10_Canister_Rifle_Infested_Duran = 113", "Dual_Photon_Blasters_Artanis = 114", "Anti_Matter_Missiles_Artanis = 115", "C_10_Canister_Rifle_Alexei_Stukov = 116", "None = 130", "Unknown", "MAX", }; constexpr const char * const * _raw_names() { return _the_raw_names; } inline char* _name_storage() { static char storage[] = "Gauss_Rifle = 0" "," "Gauss_Rifle_Jim_Raynor = 1" "," "C_10_Canister_Rifle = 2" "," "C_10_Canister_Rifle_Sarah_Kerrigan = 3" "," "Fragmentation_Grenade = 4" "," "Fragmentation_Grenade_Jim_Raynor = 5" "," "Spider_Mines = 6" "," "Twin_Autocannons = 7" "," "Hellfire_Missile_Pack = 8" "," "Twin_Autocannons_Alan_Schezar = 9" "," "Hellfire_Missile_Pack_Alan_Schezar = 10" "," "Arclite_Cannon = 11" "," "Arclite_Cannon_Edmund_Duke = 12" "," "Fusion_Cutter = 13" "," "Gemini_Missiles = 15" "," "Burst_Lasers = 16" "," "Gemini_Missiles_Tom_Kazansky = 17" "," "Burst_Lasers_Tom_Kazansky = 18" "," "ATS_Laser_Battery = 19" "," "ATA_Laser_Battery = 20" "," "ATS_Laser_Battery_Hero = 21" "," "ATA_Laser_Battery_Hero = 22" "," "ATS_Laser_Battery_Hyperion = 23" "," "ATA_Laser_Battery_Hyperion = 24" "," "Flame_Thrower = 25" "," "Flame_Thrower_Gui_Montag = 26" "," "Arclite_Shock_Cannon = 27" "," "Arclite_Shock_Cannon_Edmund_Duke = 28" "," "Longbolt_Missile = 29" "," "Yamato_Gun = 30" "," "Nuclear_Strike = 31" "," "Lockdown = 32" "," "EMP_Shockwave = 33" "," "Irradiate = 34" "," "Claws = 35" "," "Claws_Devouring_One = 36" "," "Claws_Infested_Kerrigan = 37" "," "Needle_Spines = 38" "," "Needle_Spines_Hunter_Killer = 39" "," "Kaiser_Blades = 40" "," "Kaiser_Blades_Torrasque = 41" "," "Toxic_Spores = 42" "," "Spines = 43" "," "Acid_Spore = 46" "," "Acid_Spore_Kukulza = 47" "," "Glave_Wurm = 48" "," "Glave_Wurm_Kukulza = 49" "," "Seeker_Spores = 52" "," "Subterranean_Tentacle = 53" "," "Suicide_Infested_Terran = 54" "," "Suicide_Scourge = 55" "," "Parasite = 56" "," "Spawn_Broodlings = 57" "," "Ensnare = 58" "," "Dark_Swarm = 59" "," "Plague = 60" "," "Consume = 61" "," "Particle_Beam = 62" "," "Psi_Blades = 64" "," "Psi_Blades_Fenix = 65" "," "Phase_Disruptor = 66" "," "Phase_Disruptor_Fenix = 67" "," "Psi_Assault = 69" "," "Psionic_Shockwave = 70" "," "Psionic_Shockwave_TZ_Archon = 71" "," "Dual_Photon_Blasters = 73" "," "Anti_Matter_Missiles = 74" "," "Dual_Photon_Blasters_Mojo = 75" "," "Anti_Matter_Missiles_Mojo = 76" "," "Phase_Disruptor_Cannon = 77" "," "Phase_Disruptor_Cannon_Danimoth = 78" "," "Pulse_Cannon = 79" "," "STS_Photon_Cannon = 80" "," "STA_Photon_Cannon = 81" "," "Scarab = 82" "," "Stasis_Field = 83" "," "Psionic_Storm = 84" "," "Warp_Blades_Zeratul = 85" "," "Warp_Blades_Hero = 86" "," "Platform_Laser_Battery = 92" "," "Independant_Laser_Battery = 93" "," "Twin_Autocannons_Floor_Trap = 96" "," "Hellfire_Missile_Pack_Wall_Trap = 97" "," "Flame_Thrower_Wall_Trap = 98" "," "Hellfire_Missile_Pack_Floor_Trap = 99" "," "Neutron_Flare = 100" "," "Disruption_Web = 101" "," "Restoration = 102" "," "Halo_Rockets = 103" "," "Corrosive_Acid = 104" "," "Mind_Control = 105" "," "Feedback = 106" "," "Optical_Flare = 107" "," "Maelstrom = 108" "," "Subterranean_Spines = 109" "," "Warp_Blades = 111" "," "C_10_Canister_Rifle_Samir_Duran = 112" "," "C_10_Canister_Rifle_Infested_Duran = 113" "," "Dual_Photon_Blasters_Artanis = 114" "," "Anti_Matter_Missiles_Artanis = 115" "," "C_10_Canister_Rifle_Alexei_Stukov = 116" "," "None = 130" "," "Unknown" "," "MAX" ","; return storage; } inline const char** _name_array() { static const char *value[WeaponType::_size_constant]; return value; } inline bool& _initialized() { static bool value = false; return value; } }  constexpr inline const WeaponType operator +(WeaponType::_enumerated enumerated) { return static_cast<WeaponType>(enumerated); } constexpr inline WeaponType::_optional_index WeaponType::_from_value_loop(WeaponType::_integral value, std::size_t index) { return index == _size() ? _optional_index() : better_enums_data_WeaponType::_value_array[index]._value == value ? _optional_index(index) : _from_value_loop(value, index + 1); } constexpr inline WeaponType::_optional_index WeaponType::_from_string_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match(better_enums_data_WeaponType::_raw_names()[index], name) ? _optional_index(index) : _from_string_loop(name, index + 1); } constexpr inline WeaponType::_optional_index WeaponType::_from_string_nocase_loop(const char *name, std::size_t index) { return index == _size() ? _optional_index() : ::better_enums::_names_match_nocase(better_enums_data_WeaponType::_raw_names()[index], name) ? _optional_index(index) : _from_string_nocase_loop(name, index + 1); } constexpr inline WeaponType::_integral WeaponType::_to_integral() const { return _integral(_value); } constexpr inline WeaponType WeaponType::_from_integral_unchecked(_integral value) { return static_cast<_enumerated>(value); } constexpr inline WeaponType::_optional WeaponType::_from_integral_nothrow(_integral value) { return ::better_enums::_map_index<WeaponType>(better_enums_data_WeaponType::_value_array, _from_value_loop(value)); } constexpr inline WeaponType WeaponType::_from_integral(_integral value) { return ::better_enums::_or_throw(_from_integral_nothrow(value), "WeaponType" "::_from_integral: invalid argument"); } inline const char* WeaponType::_to_string() const { return ::better_enums::_or_null(::better_enums::_map_index<const char*>(better_enums_data_WeaponType::_name_array(), _from_value_loop(::better_enums::continue_with(initialize(), _value)))); } constexpr inline WeaponType::_optional WeaponType::_from_string_nothrow(const char *name) { return ::better_enums::_map_index<WeaponType>(better_enums_data_WeaponType::_value_array, _from_string_loop(name)); } constexpr inline WeaponType WeaponType::_from_string(const char *name) { return ::better_enums::_or_throw(_from_string_nothrow(name), "WeaponType" "::_from_string: invalid argument"); } constexpr inline WeaponType::_optional WeaponType::_from_string_nocase_nothrow(const char *name) { return ::better_enums::_map_index<WeaponType>(better_enums_data_WeaponType::_value_array, _from_string_nocase_loop(name)); } constexpr inline WeaponType WeaponType::_from_string_nocase(const char *name) { return ::better_enums::_or_throw(_from_string_nocase_nothrow(name), "WeaponType" "::_from_string_nocase: invalid argument"); } constexpr inline bool WeaponType::_is_valid(_integral value) { return _from_value_loop(value); } constexpr inline bool WeaponType::_is_valid(const char *name) { return _from_string_loop(name); } constexpr inline bool WeaponType::_is_valid_nocase(const char *name) { return _from_string_nocase_loop(name); } constexpr inline const char* WeaponType::_name() { return "WeaponType"; } constexpr inline WeaponType::_value_iterable WeaponType::_values() { return _value_iterable(better_enums_data_WeaponType::_value_array, _size()); } inline WeaponType::_name_iterable WeaponType::_names() { return _name_iterable(better_enums_data_WeaponType::_name_array(), ::better_enums::continue_with(initialize(), _size())); } inline int WeaponType::initialize() { if (better_enums_data_WeaponType::_initialized()) return 0; ::better_enums::_trim_names(better_enums_data_WeaponType::_raw_names(), better_enums_data_WeaponType::_name_array(), better_enums_data_WeaponType::_name_storage(), _size()); better_enums_data_WeaponType::_initialized() = true; return 0; }  constexpr inline bool operator ==(const WeaponType &a, const WeaponType &b) { return a._to_integral() == b._to_integral(); }  constexpr inline bool operator !=(const WeaponType &a, const WeaponType &b) { return a._to_integral() != b._to_integral(); }  constexpr inline bool operator <(const WeaponType &a, const WeaponType &b) { return a._to_integral() < b._to_integral(); }  constexpr inline bool operator <=(const WeaponType &a, const WeaponType &b) { return a._to_integral() <= b._to_integral(); }  constexpr inline bool operator >(const WeaponType &a, const WeaponType &b) { return a._to_integral() > b._to_integral(); }  constexpr inline bool operator >=(const WeaponType &a, const WeaponType &b) { return a._to_integral() >= b._to_integral(); } template <typename Char, typename Traits> std::basic_ostream<Char, Traits>& operator <<(std::basic_ostream<Char, Traits>& stream, const WeaponType &value) { return stream << value._to_string(); } template <typename Char, typename Traits> std::basic_istream<Char, Traits>& operator >>(std::basic_istream<Char, Traits>& stream, WeaponType &value) { std::basic_string<Char, Traits> buffer; stream >> buffer; ::better_enums::optional<WeaponType> converted = WeaponType::_from_string_nothrow(buffer.c_str()); if (converted) value = *converted; else stream.setstate(std::basic_istream<Char, Traits>::failbit); return stream; }
#endif // TC_EXPAND_LARGE_ENUMS

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
    Race,
    int,
    // corresponds to BWAPI::Races::Enum
    Zerg = 0,
    Terran = 1,
    Protoss = 2,
    Other = 3,
    Unused = 4,
    Select = 5,
    Random = 6,
    None = 7,
    Unknown = 8,
    MAX)

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
