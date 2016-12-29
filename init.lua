local torchcraft = require 'torchcraft._env'
local utils = require 'torchcraft.utils'
local replayer = require 'torchcraft.replayer'
local tablex = require 'pl.tablex'
local image = require 'image'

-- All available commands
local vars = {
    -- no arguments
    quit = 0,                  -- leave the game
    restart = 1,               -- resetart the game. Much faster, but doesn't
                               --   work in multiplayer.
    map_hack = 2,              -- remove fog of war
    request_image = 3,
    exit_process = 4,
    noop = 5,                  -- do nothing

    -- one argument
    set_speed = 6,             -- sets the game speed (integer)
    set_log = 7,               -- activates logging (boolean)
    set_gui = 8,               -- activates drawing and text in SC (boolean)
    set_frameskip = 9,         -- number of frames to skip (integer)
    set_cmd_optim = 10,        -- reduce bot APM (0-6)
    set_combine_frames = 11,   -- combine n frames before sending (integer)

      -- Sets the map with BWAPI->setMap and by writing to the config. Is not
      --   thread-safe. However, as long as the next connect finishes after
      --   set_map, you are guaranteed the map will be what you want.
    set_map = 12,
    set_multi = 13,

    -- arguments: (unit ID, command, target id, target x, target y, extra)
    -- (x, y) are walktiles instead of pixels
    -- otherwise this corresponds exactly to BWAPI::UnitCommand
    command_unit = 14,
    command_unit_protected = 15,

    -- arguments: (command, args)
    -- For documentation about args, see usercommandtypes
    command_user = 16,

    --
    MAX_ACTION = 17
}

for k,v in pairs(vars) do
    torchcraft[k] = v
end

local function seal(t)
    local mt = {}
    mt.__newindex = function(tab, k, v)
        error('Attempting to add a field to sealed table', 2)
    end
    setmetatable(t, mt)
end

local function build_revert_indices_and_seal(t)
    do -- build reverse indices
       local tmp = {}
       for k, v in pairs(t) do
          table.insert(tmp, k)
       end
       for _, k in pairs(tmp) do
          t[t[k]] = k
       end
    end
    seal(t)
end

torchcraft.usercommandtypes = {
  -- one arg
  Move_Screen_Up = 0,       -- arguments: magnitude (amount of pixels)
  Move_Screen_Down = 1,     -- arguments: magnitude (amount of pixels)
  Move_Screen_Left = 2,     -- arguments: magnitude (amount of pixels)
  Move_Screen_Right = 3,    -- arguments: magnitude (amount of pixels)

  -- two args
  Move_Screen_To_Pos = 4,   -- arguments: (x, y)
  Right_Click = 5,          -- arguments: (x, y)

  --
  USER_COMMAND_END = 7
}
build_revert_indices_and_seal(torchcraft.usercommandtypes)

torchcraft.usercmd = torchcraft.usercommandtypes   -- shortcut

-- static table, is sealed after initialization
torchcraft.unitcommandtypes = {
    -- corresponds to BWAPI::UnitCommandTypes::Enum
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
    MAX = 46
}
build_revert_indices_and_seal(torchcraft.unitcommandtypes)

torchcraft.cmd = torchcraft.unitcommandtypes    -- shortcut

-- static table, is sealed after initialization
torchcraft.orders = {
    -- corresponds to BWAPI::Orders::Enum
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
    MAX = 191
}
build_revert_indices_and_seal(torchcraft.orders)

-- static table, is sealed after initialization
torchcraft.techtypes = {
    -- corresponds to BWAPI::TechTypes::Enum
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
    MAX = 47,
}
build_revert_indices_and_seal(torchcraft.techtypes)

-- static table, is sealed after initialization
torchcraft.unittypes = {
    -- corresponds to BWAPI::UnitTypes::Enum
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
    MAX = 233,
}
build_revert_indices_and_seal(torchcraft.unittypes)

function torchcraft:isbuilding(unittypeid)
    return unittypeid >= torchcraft.unittypes.Terran_Command_Center and
        unittypeid <= torchcraft.unittypes.Protoss_Shield_Battery
end

function torchcraft:isworker(unittypeid)
    return unittypeid == torchcraft.unittypes.Protoss_Probe or
        unittypeid == torchcraft.unittypes.Terran_SCV or
        unittypeid == torchcraft.unittypes.Zerg_Drone
end

function torchcraft:is_mineral_field(unittypeid)
    return unittypeid == self.unittypes.Resource_Mineral_Field or
        unittypeid == self.unittypes.Resource_Mineral_Field_Type_2 or
        unittypeid == self.unittypes.Resource_Mineral_Field_Type_3
end

function torchcraft:is_gas_geyser(unittypeid)
    return unittypeid == self.unittypes.Resource_Vespene_Geyser or
        unittypeid == self.unittypes.Protoss_Assimilator or
        unittypeid == self.unittypes.Terran_Refinery or
        unittypeid == self.unittypes.Zerg_Extractor
end

-- static table, is sealed after initialization
torchcraft.produces = { -- a helpful approximation (e.g. bypasses Eggs)
    -- TODO remove when port to C++, use BWAPI directly for this
    [torchcraft.unittypes.Terran_Vulture] =
        {torchcraft.unittypes.Terran_Vulture_Spider_Mine},
    [torchcraft.unittypes.Terran_SCV] = (function()
        r = {}
        for i = torchcraft.unittypes.Terran_Command_Center,
            torchcraft.unittypes.Terran_Bunker do table.insert(r, i) end
        return r
    end)(),
    [torchcraft.unittypes.Zerg_Larva] = (function()
        r = {}
        for i = torchcraft.unittypes.Zerg_Zergling,
            torchcraft.unittypes.Zerg_Scourge do
            if i ~= torchcraft.unittypes.Broodling and
               i ~= torchcraft.unittypes.Guardian then
                table.insert(r, i)
            end
        end
        return r
    end)(),
    [torchcraft.unittypes.Zerg_Queen] =
        {torchcraft.unittypes.Zerg_Broodling,
         torchcraft.unittypes.Zerg_Infested_Command_Center},
    [torchcraft.unittypes.Zerg_Hydralisk] =
        {torchcraft.unittypes.Zerg_Lurker},
    [torchcraft.unittypes.Zerg_Drone] = (function()
        r = {}
        for i = torchcraft.unittypes.Zerg_Hatchery,
            torchcraft.unittypes.Zerg_Extractor do table.insert(r, i) end
        return r
    end)(),
    [torchcraft.unittypes.Zerg_Mutalisk] =
        {torchcraft.unittypes.Zerg_Guardian,
         torchcraft.unittypes.Zerg_Devourer},
    [torchcraft.unittypes.Protoss_Probe] = (function()
        r = {}
        for i = torchcraft.unittypes.Protoss_Nexus,
            torchcraft.unittypes.Protoss_Shield_Battery
            do
                table.insert(r, i)
            end
        return r
    end)(),
    [torchcraft.unittypes.Protoss_Carrier] =
        {torchcraft.unittypes.Protoss_Interceptor},
    [torchcraft.unittypes.Protoss_Reaver] =
        {torchcraft.unittypes.Protoss_Scarab},
    [torchcraft.unittypes.Terran_Nuclear_Silo] =
        {torchcraft.unittypes.Terran_Nuclear_Missile},
    [torchcraft.unittypes.Terran_Barracks] =
        {torchcraft.unittypes.Terran_Marine,
         torchcraft.unittypes.Terran_Firebat,
         torchcraft.unittypes.Terran_Medic},
    [torchcraft.unittypes.Terran_Factory] =
        {torchcraft.unittypes.Terran_Vulture,
         torchcraft.unittypes.Terran_Siege_Tank_Tank_Mode,
         torchcraft.unittypes.Terran_Goliath},
    [torchcraft.unittypes.Terran_Starport] =
        {torchcraft.unittypes.Terran_Wraith,
         torchcraft.unittypes.Terran_Valkyrie,
         torchcraft.unittypes.Terran_Science_Vessel,
         torchcraft.unittypes.Terran_Dropship,
         torchcraft.unittypes.Terran_Battlecruiser},
    [torchcraft.unittypes.Zerg_Infested_Command_Center] =
        {torchcraft.unittypes.Zerg_Infested_Terran},
    [torchcraft.unittypes.Zerg_Hatchery] =
        {torchcraft.unittypes.Zerg_Larva,
         torchcraft.unittypes.Zerg_Lair},
    [torchcraft.unittypes.Zerg_Lair] =
        {torchcraft.unittypes.Zerg_Larva,
         torchcraft.unittypes.Zerg_Hive},
    [torchcraft.unittypes.Zerg_Hive] =
        {torchcraft.unittypes.Zerg_Larva},
    [torchcraft.unittypes.Zerg_Creep_Colony] =
        {torchcraft.unittypes.Zerg_Spore_Colony,
         torchcraft.unittypes.Zerg_Sunken_Colony},
    [torchcraft.unittypes.Protoss_Nexus] =
        {torchcraft.unittypes.Protoss_Probe},
    [torchcraft.unittypes.Protoss_Robotics_Facility] =
        {torchcraft.unittypes.Protoss_Reaver,
         torchcraft.unittypes.Protoss_Observer,
         torchcraft.unittypes.Protoss_Shuttle},
    [torchcraft.unittypes.Protoss_High_Templar] =
        {torchcraft.unittypes.Protoss_Archon},
    [torchcraft.unittypes.Protoss_Dark_Templar] =
        {torchcraft.unittypes.Protoss_Dark_Archon},
    [torchcraft.unittypes.Protoss_Gateway] =
        {torchcraft.unittypes.Protoss_Zealot,
         torchcraft.unittypes.Protoss_Dragoon,
         torchcraft.unittypes.Protoss_High_Templar,
         torchcraft.unittypes.Protoss_Dark_Templar},
    [torchcraft.unittypes.Protoss_Stargate] =
        {torchcraft.unittypes.Protoss_Scout,
         torchcraft.unittypes.Protoss_Corsair,
         torchcraft.unittypes.Protoss_Carrier},
}
seal(torchcraft.produces)

-- static table, is sealed after initialization
torchcraft.isproducedby = {} -- a helpful (inverse) approximation
-- TODO remove when port to C++, use BWAPI directly for this
for producer, products in pairs(torchcraft.produces) do
    seal(products)
    for _, product in pairs(products) do
        torchcraft.isproducedby[product] = producer
    end
end
seal(torchcraft.isproducedby)

-- static table, is sealed after initialization
torchcraft.bullettypes = {
    -- corresponds to BWAPI::BulletTypes::Enum
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
    MAX = 211,
}
build_revert_indices_and_seal(torchcraft.bullettypes)

-- static table, is sealed after initialization
torchcraft.weapontypes = {
    -- corresponds to BWAPI::WeaponTypes::Enum
"Gauss_Rifle", "Gauss_Rifle_Jim_Raynor",
"C_10_Canister_Rifle", "C_10_Canister_Rifle_Sarah_Kerrigan",
"Fragmentation_Grenade", "Fragmentation_Grenade_Jim_Raynor", "Spider_Mines",
"Twin_Autocannons", "Hellfire_Missile_Pack", "Twin_Autocannons_Alan_Schezar",
"Hellfire_Missile_Pack_Alan_Schezar", "Arclite_Cannon",
"Arclite_Cannon_Edmund_Duke", "Fusion_Cutter", "", "Gemini_Missiles",
"Burst_Lasers", "Gemini_Missiles_Tom_Kazansky", "Burst_Lasers_Tom_Kazansky",
"ATS_Laser_Battery", "ATA_Laser_Battery", "ATS_Laser_Battery_Hero",
"ATA_Laser_Battery_Hero", "ATS_Laser_Battery_Hyperion",
"ATA_Laser_Battery_Hyperion", "Flame_Thrower", "Flame_Thrower_Gui_Montag",
"Arclite_Shock_Cannon", "Arclite_Shock_Cannon_Edmund_Duke", "Longbolt_Missile",
"Yamato_Gun", "Nuclear_Strike", "Lockdown", "EMP_Shockwave", "Irradiate",
"Claws", "Claws_Devouring_One", "Claws_Infested_Kerrigan", "Needle_Spines",
"Needle_Spines_Hunter_Killer", "Kaiser_Blades", "Kaiser_Blades_Torrasque",
"Toxic_Spores", "Spines", "", "", "Acid_Spore", "Acid_Spore_Kukulza",
"Glave_Wurm", "Glave_Wurm_Kukulza", "", "", "Seeker_Spores",
"Subterranean_Tentacle", "Suicide_Infested_Terran", "Suicide_Scourge",
"Parasite", "Spawn_Broodlings", "Ensnare", "Dark_Swarm", "Plague", "Consume",
"Particle_Beam", "", "Psi_Blades", "Psi_Blades_Fenix", "Phase_Disruptor",
"Phase_Disruptor_Fenix", "", "Psi_Assault", "Psionic_Shockwave",
"Psionic_Shockwave_TZ_Archon", "", "Dual_Photon_Blasters",
"Anti_Matter_Missiles", "Dual_Photon_Blasters_Mojo",
"Anti_Matter_Missiles_Mojo", "Phase_Disruptor_Cannon",
"Phase_Disruptor_Cannon_Danimoth", "Pulse_Cannon", "STS_Photon_Cannon",
"STA_Photon_Cannon", "Scarab", "Stasis_Field", "Psionic_Storm",
"Warp_Blades_Zeratul", "Warp_Blades_Hero", "", "", "", "", "",
"Platform_Laser_Battery", "Independant_Laser_Battery", "", "",
"Twin_Autocannons_Floor_Trap", "Hellfire_Missile_Pack_Wall_Trap",
"Flame_Thrower_Wall_Trap", "Hellfire_Missile_Pack_Floor_Trap", "Neutron_Flare",
"Disruption_Web", "Restoration", "Halo_Rockets", "Corrosive_Acid",
"Mind_Control", "Feedback", "Optical_Flare", "Maelstrom",
"Subterranean_Spines", "", "Warp_Blades", "C_10_Canister_Rifle_Samir_Duran",
"C_10_Canister_Rifle_Infested_Duran", "Dual_Photon_Blasters_Artanis",
"Anti_Matter_Missiles_Artanis", "C_10_Canister_Rifle_Alexei_Stukov", "", "",
"", "", "", "", "", "", "", "", "", "", "", "None", "Unknown"
}
build_revert_indices_and_seal(torchcraft.weapontypes)

-- static table, is sealed after initialization
torchcraft.unitsizes = {
    -- corresponds to BWAPI::UnitSizeTypes::Enum
    Independent = 0,
    Small = 1,
    Medium = 2,
    Large = 3,
}
build_revert_indices_and_seal(torchcraft.unitsizes)

-- static table, is sealed after initialization
torchcraft.dmgtypes = {
    -- corresponds to BWAPI::DamageTypes::Enum
    Independent = 0,
    Explosive = 1,
    Concussive = 2,
    Normal = 3,
    Ignore_Armor = 4,
    None = 5,
}
build_revert_indices_and_seal(torchcraft.dmgtypes)

local c = torchcraft.unitcommandtypes
local o = torchcraft.orders
torchcraft.command2order = {
    -- corresponds to BWAPI::UnitCommandTypes to BWAPI::Orders
    [c.Halt_Construction] = {o.ResetCollision},
    [c.Upgrade] = {o.Upgrade},
    [c.Cancel_Morph] = {o.PlayerGuard, o.ResetCollision},
    [c.Return_Cargo] = {o.ReturnGas, o.ReturnMinerals, o.ResetCollision},
    [c.Attack_Unit] = {o.AttackUnit, o.InterceptorAttack, o.ScarabAttack},
    [c.Cloak] = {o.Cloak},
    [c.Research] = {o.ResearchTech},
    [c.Attack_Move] = {o.AttackMove},
    [c.Build] = {o.PlaceBuilding, o.BuildNydusExit, o.CreateProtossBuilding},
    [c.Right_Click_Unit] = {o.MoveToMinerals, o.MoveToGas,
        o.ConstructingBuilding, o.AttackUnit, o.Follow, o.ResetCollision,
        o.EnterNydusCanal, o.EnterTransport, o.Harvest1, o.Harvest2,
        o.Harvest3, o.Harvest4, o.InterceptorAttack, o.HarvestGas,
        o.MedicHeal, o.MiningMinerals, o.ReturnMinerals, o.ReturnGas,
        o.RightClickAction, o.ScarabAttack, o.WaitForGas, o.WaitForMinerals},
    [c.Cancel_Upgrade] = {o.Nothing},
    [c.Siege] = {o.Sieging},
    [c.Train] = {o.Train, o.TrainFighter},
    [c.Unload] = {o.Unload},
    [c.Stop] = {o.Stop, o.Interrupted},
    [c.Cancel_Research] = {o.Nothing},
    [c.Lift] = {o.BuildingLiftOff},
    [c.Unburrow] = {o.Unburrowing},
    [c.Cancel_Train_Slot] = {o.Nothing},
    [c.Land] = {o.BuildingLand},
    [c.Set_Rally_Unit] = {o.RallyPointUnit},
    [c.Hold_Position] = {o.HoldPosition},
    [c.Morph] = {o.ZergUnitMorph, o.ZergBuildingMorph},
    [c.Cancel_Construction] = {o.ResetCollision, o.Die},
    [c.Gather] = {o.MoveToMinerals, o.MoveToGas, o.Harvest1, o.Harvest2,
        o.Harvest3, o.Harvest4, o.HarvestGas, o.MiningMinerals,
        o.WaitForGas, o.WaitForMinerals, o.ResetCollision, o.ReturnMinerals},
    [c.Cancel_Addon] = {o.Nothing},
    [c.Cancel_Train] = {o.Nothing},
    [c.Burrow] = {o.Burrowing},
    [c.Decloak] = {o.Decloak},
    [c.Unsiege] = {o.Unsieging},
    [c.Right_Click_Position] = {o.Move},
    [c.Unload_All] = {o.Unload, o.MoveUnload},
    [c.Load] = {o.PickupBunker, o.PickupTransport, o.EnterTransport, o.Pickup4},
    [c.Repair] = {o.Repair},
    [c.Unload_All_Position] = {o.MoveUnload},
    [c.Patrol] = {o.Patrol},
    [c.Move] = {o.Move},
    [c.Build_Addon] = {o.BuildAddon, o.PlaceAddon},
    [c.Set_Rally_Position] = {o.RallyPointTile, o.RallyPointUnit},
    [c.Follow] = {o.Follow},
    [c.Use_Tech] = {o.Cloak, o.Decloak},
    [c.Use_Tech_Position] = {o.CastDarkSwarm, o.CastDisruptionWeb,
        o.CastEMPShockwave, o.CastEnsnare, o.CastNuclearStrike, o.CastRecall,
        o.CastPsionicStorm, o.CastPlague, o.CastScannerSweep,
        o.CastStasisField, o.PlaceMine},
    [c.Use_Tech_Unit] = {o.ArchonWarp, o.CastConsume, o.CastDefensiveMatrix,
        o.CastFeedback, o.CastHallucination, o.CastIrradiate,
        o.CastInfestation, o.CastLockdown, o.CastMaelstrom, o.CastMindControl,
        o.CastOpticalFlare, o.CastParasite, o.CastRestoration,
        o.CastSpawnBroodlings, o.DarkArchonMeld, o.FireYamatoGun,
        o.InfestingCommandCenter, o.RechargeShieldsUnit}
}
seal(torchcraft.command2order)
torchcraft.order2command = {}
-- Most of unknown orders to Idle/Stop
for command, orders in pairs(torchcraft.command2order) do
    for _, order in pairs(orders) do
        if torchcraft.order2command[order] == nil then
            torchcraft.order2command[order] = {}
        end
        table.insert(torchcraft.order2command[order], command)
    end
end
seal(torchcraft.order2command)

torchcraft.xy_pixels_per_walktile = 8
torchcraft.xy_pixels_per_buildtile = 32
torchcraft.xy_walktiles_per_buildtile = 4
torchcraft.hit_prob_ranged_uphill_doodad = 0.53125
torchcraft.hit_prob_ranged = 0.99609375

do -- load and seal other static data
    local json = require 'cjson'
    torchcraft.staticdata = json.decode(io.open(                    -- TODO
                paths.thisfile('starcraft_static.json')) -- path!
                :read("*all"))
    for property, data_table in pairs(torchcraft.staticdata) do
        local add = {}
        for unittype, value in pairs(data_table) do
            if torchcraft.unittypes[unittype] ~= nil then
                add[torchcraft.unittypes[unittype]] = value
            end
        end
        for k, v in pairs(add) do
            data_table[k] = v
        end
    end
   local tmp = {}
   tmp.__newindex = function(t, k, v)
      error('Attempting to add a field to sealed table torchcraft.staticdata')
   end
   setmetatable(torchcraft.staticdata, tmp)
end

torchcraft.total_price = {mineral = {}, gas = {}}
for ut, _ in pairs(torchcraft.produces) do
    torchcraft.total_price.mineral[ut] = torchcraft.staticdata.mineralPrice[ut]
    torchcraft.total_price.gas[ut] = torchcraft.staticdata.gasPrice[ut]
end
for ut, producer in pairs(torchcraft.isproducedby) do -- only 1 hop ever
    if torchcraft:isbuilding(producer) or
        producer == torchcraft.unittypes.Zerg_Larva then
        torchcraft.total_price.mineral[ut] = torchcraft.staticdata.mineralPrice[ut]
        torchcraft.total_price.gas[ut] = torchcraft.staticdata.gasPrice[ut]
    elseif ut == torchcraft.unittypes.Protoss_Archon
        or ut == torchcraft.unittypes.Protoss_Dark_Archon then
        torchcraft.total_price.mineral[ut] =
            2 * torchcraft.staticdata.mineralPrice[producer]
        torchcraft.total_price.gas[ut] =
            2 * torchcraft.staticdata.gasPrice[producer]
    else
        torchcraft.total_price.mineral[ut] =
            (torchcraft.staticdata.mineralPrice[ut] or 0)
            + torchcraft.staticdata.mineralPrice[producer]
        torchcraft.total_price.gas[ut] =
            (torchcraft.staticdata.gasPrice[ut] or 0)
            + torchcraft.staticdata.gasPrice[producer]
    end
end
seal(torchcraft.total_price)
seal(torchcraft.total_price.mineral)
seal(torchcraft.total_price.gas)
assert(torchcraft.total_price.mineral[torchcraft.unittypes.Zerg_Guardian]
    == torchcraft.staticdata.mineralPrice[torchcraft.unittypes.Zerg_Mutalisk]
       + torchcraft.staticdata.mineralPrice[torchcraft.unittypes.Zerg_Guardian])
assert(torchcraft.total_price.mineral[torchcraft.unittypes.Protoss_Dark_Archon]
    == 2 * torchcraft.staticdata.mineralPrice[torchcraft.unittypes.Protoss_Dark_Templar])
assert(torchcraft.total_price.gas[torchcraft.unittypes.Terran_Science_Vessel]
    == 225)


torchcraft.PROTOCOL_VERSION = "16"
torchcraft.hostname = nil
torchcraft.state = {}
torchcraft.mode = {micro_battles = false, replay = false}
torchcraft.DEBUG = 0
torchcraft.initial_map = nil
torchcraft.window_size = nil
torchcraft.window_pos = nil
torchcraft.field_size = {640, 370}   -- size of the field view in pixels (approximately)
--[[
    state will get its content updated from bwapi, it will have
    * map_data            : [torch.ByteTensor] 2D. 255 (-1) where not walkable
    * map_name            : [string] Name on the current map
    * img_mode            : [string] Image mode selected (can be empty, raw, compress)
    * lag_frames          : [int] number of frames from order to execution
    * frame_from_bwapi    : [int] game frame number as seen from BWAPI
    * game_ended          : [boolean] did the game end? (i.e. did the map end)
    * battle_just_ended   : [boolean] did the battle just end? (battle!=game)
    * waiting_for_restart : [boolean] are we waiting to restart a new battle?
    * battle_won          : [boolean] did we win the battle?
    * units_myself        : [table] w/ {unitIDs: unitStates} as {keys: values}
    * units_enemy         : [table] same as above, but for the enemy player
    * bullets             : [table] table with all bullets (position and type)
    * screen_position     : [table] Position of screen {x, y} in pixels. {0, 0} is top-left
]]

function torchcraft:init(hostname, port)
    if hostname == '' or hostname == nil then
        -- this is the local VM when running e.g. on a laptop + VMware windows
        -- known problem: sometimes this arp command fails on VPNs...
        local arpstring = 'arp -a -i vmnet8 | grep -v incomplete | '
                          .. 'tail -1 | cut -f2 -d" " | tr -d "()"'
        print("executing: " .. arpstring)
        self.hostname = sys.fexecute(arpstring)
    else
        self.hostname = hostname
    end
    self.port = port ~= nil and port or (os.getenv('TorchCraft_PORT') or 11111)
    print('host: ' .. self.hostname .. ':' .. self.port)
    -- we always need to make sure we alternate 1:1 send/receive, thus:
    self.sent_message = false
end

function torchcraft:connect(port)
    -- connect() should be called at the beginning of every game
    if self.hostname == nil or self.hostname == '' then
        self:init(nil, port)
    end
    -- initialize ZMQ if needed
    if self.zmq == nil then
        self.zmq = require 'lzmq'
    end
    if self.zcontext == nil then
        self.zcontext = self.zmq.context()
    end
    -- initialize socket connection
    self.sock = nil
    while self.sock == nil do
        local addr = 'tcp://' .. self.hostname .. ':' .. self.port
        self.sock, self.err = self.zcontext:socket{self.zmq.REQ,
            connect = addr}
        if self.sock == nil then
            print('Socket error (' .. addr ..
                  '), retrying connection in 1 second: ', self.err)
            os.execute('sleep 1')
        end
    end

    self.state = {}

    -- send hello message
    local hello = 'protocol=' .. self.PROTOCOL_VERSION

    if self.initial_map then
        hello = hello .. ",map=" .. self.initial_map
    end

    if self.window_size then
        hello = hello .. ",window_size=" .. self.window_size[1] .. " " .. self.window_size[2]
    end

    if self.window_pos then
        hello = hello .. ",window_pos=" .. self.window_pos[1] .. " " .. self.window_pos[2]
    end

    hello = hello .. ",micro_mode=" .. tostring(self.mode.micro_battles)

    local ok, err = self.sock:send(hello)
    if not ok then
        error('tc.connect send protocol: '..err:name()..' '..err:msg())
    end

    -- receive setup message
    local msg, more = self.sock:recv()
    if not msg then
        local err = more
        error('tc.connect receive setup: '..err:name()..' '..err:msg())
    end
    assert(not more, "Expected single message to be received.")

    local setup = loadstring('return ' .. msg)()
    for k, v in pairs(setup) do
        self.state[k] = v
    end

    self.sent_message = false
    if self.DEBUG > 0 then
        print('torchcraft:connect() finished, establishing command control.')
    end

    return setup
end

function torchcraft:set_variables()
    -- initializing the "game state" booleans
    self.state.game_ended = false
    self.state.img_mode = nil
    self.state.screen_position = nil
    self.state.window_size = {200, 150}
    self.state.image = nil
    if self.mode.micro_battles then
        self.state.battle_just_ended = false
        self.state.battle_won = false
        self.state.waiting_for_restart = true
        self.state.last_battle_end = 0
        self.state.frame_from_bwapi = 0
    end
    if self.mode.replay then
        self.state.units = {}
        self.state.resources = {}
    else
        self.state.units_myself = {}
        self.state.units_enemy = {}
        self.state.resources_myself = {}
    end
end

function torchcraft:battle_ended()
    self.state.battle_just_ended = true
    self.state.waiting_for_restart = true
    self.state.last_battle_ended = self.state.frame_from_bwapi
end

function torchcraft:filter_units_table(t)
    --[[
    This function deletes all the units with an unknown type. (typically map
  reavelers)
    Input:
      - t : a lua table containing the units indexed by their units ID (uid)
    Output:
      - r : same lua table without the unknown units.
    ]]
    local r = {}
    for uid, ud in pairs(t) do
        if self.unittypes[ud.type] ~= nil then
            r[uid] = ud
        end
    end
    return r
end

function torchcraft:filter_type(t, utt)
    -- This function keeps only units from `t` of one of the types in `utt`
    local r = {}
    if t == nil then return r end
    for uid, ud in pairs(t) do
        if utils.is_in(ud.type, utt) then
            r[uid] = ud
        end
    end
    return r
end

function torchcraft:receive()
    if not self.sent_message then
        if self.DEBUG > 1 then
            print('unexpectedly sending ""')
        end
        self:send({""})
    end

    if not self.sock:poll(30000) then
        -- timeout, starcraft.exe probably crashed.
        self:close()
        error("starcraft.exe crashed")
    end
    local msg, more = self.sock:recv()
    if not msg then
        local err = more
        error('tc.receive: '..err:name()..' '..err:msg())
    end
    assert(not more, "Expected single message to be received.")
    self.sent_message = false

    -- check for stop
    local symbolic_msg = ""
    init_index, end_index = string.find(msg, "TCIMAGEDATA")
    if init_index then
      -- The image string is not properly escaped, so we have to split the data.
      symbolic_msg = string.sub(msg, 1, init_index - 1) .. "}"
      img_msg = string.sub(msg, end_index+1, -3)
      if self.state.img_mode ~= nil then
          self.state.image = self:imgMsgToImage(img_msg, self.state.img_mode)
      end
    else
      symbolic_msg = msg
    end

    local upd = loadstring('return ' .. symbolic_msg)()

    for k, v in pairs(upd) do
        if k == 'frame' then
            self.state.frame_string = v
            self.state.frame = replayer.frameFromString(v)
        else
            self.state[k] = v
        end
        if k == 'is_replay' then
            assert(v == self.mode.replay,
                "mode.replay inconsistent with starcraft state")
        end
    end

    if self.mode.micro_battles then
        -- initialize flags
        self.state.battle_just_ended = false
        self.state.battle_won = nil
        -- function to check end of battle
        -- the drawback is that the first frame of the battle is lost
        local function check_battle_finished(units_myself, units_enemy)
            if self.state.waiting_for_restart then
                return false
            end
            if utils.isEmpty(units_myself) or utils.isEmpty(units_enemy) then
                self:battle_ended()
                self.state.battle_won = (not utils.isEmpty(units_myself))
                    or utils.isEmpty(units_enemy)
                return true
            end
            return false
        end

        -- apply list of deaths on old list of units
        -- so that battle ended condition can be detected every time
        if self.state.deaths ~= nil then
            for i=1, #self.state.deaths do -- make sure order is correct
                local id = self.state.deaths[i]
                if self.mode.replay then
                    self.state.units[id] = nil
                else
                    self.state.units_myself[id] = nil
                    self.state.units_enemy[id] = nil
                end
                if check_battle_finished(self.state.units_myself,
                                         self.state.units_enemy) then
                    -- ignore the killing of remaining units in that battle
                    -- this will be re-initialized anyway in the next frame
                    break
                end
            end
        end
    end

    if not self.mode.micro_battles or not self.state.battle_just_ended then
        if not self.mode.replay and self.state.frame then
            local myself = self.state.player_id
            assert(myself ~= nil, "player_id not set but not a replay either")
            self.state.units_myself
                = self:filter_units_table(self.state.frame:getUnits(myself))
            self.state.units_enemy = self.state.frame:getUnits(1 - myself)
            self.state.resources_myself = self.state.frame:getResources(myself)
            self.state.units_neutral = self.state.frame:getUnits(self.state.neutral_id)
        elseif self.state.frame then
            self.state.units = {}
            self.state.resources = {}
            for player = 0, self.state.frame:getNumPlayers() - 1 do
                if player ~= self.state.neutral_id then
                    self.state.units[player] = self.state.frame:getUnits(player)
                    self.state.resources[player] = self.state.frame:getResources(player)
                end
            end
            self.state.units_neutral = self.state.frame:getUnits(self.state.neutral_id)
        end

        if self.state.deaths ~= nil then
            for _, id in pairs(self.state.deaths) do
                if self.mode.replay then
                    self.state.units[id] = nil
                else
                    self.state.units_myself[id] = nil
                    self.state.units_enemy[id] = nil
                end
            end
            self.state.deaths = nil
        end
        if self.mode.micro_battles and self.state.waiting_for_restart then
            local ee = utils.isEmpty(self.state.units_enemy)
            -- check if table is empty
            local we = utils.isEmpty(self.state.units_myself)
            if (not ee) and (not we) then -- we both have units
                self.state.waiting_for_restart = false
            end
        end
    end
    return upd -- for debug purposes
end

function torchcraft.command(command, ...)
    assert(command ~= nil, "Invalid command (typo?)")
    return command..","..table.concat(table.pack(...), ",")
end

function torchcraft.is_action(action, command)
    assert(action[1] ~= nil, "Invalid action check (not an action!)")
    assert(command ~= nil, "Invalid action check (typo?)")
    return (action[1] == torchcraft.command_unit
                or action[1] == torchcraft.command_unit_protected)
           and action[3] == command
end

function torchcraft.same_order(o1, o2)
    return o1.type == o2.type and o1.target == o2.target
        and o1.targetpos[1] == o2.targetpos[1]
        and o1.targetpos[2] == o2.targetpos[2]
end

function torchcraft:send(t)
    if self.sent_message then
        local tmp_upd = self:receive()
        if self.DEBUG > 1 then
            print('unexpectedly received', tmp_upd)
        end
    end

    if type(t) == "table" then
        t = table.concat(t, ":")
    end

    local ok, err = self.sock:send(t)
    if not ok then
        error('tc.send: '..err:name()..' '..err:msg())
    end
    self.sent_message = true
end

function torchcraft:close()
    self.sock:close()
end

-- return a new torchcraft context
function torchcraft.new()
   local newtc = {}
   for k,v in pairs(torchcraft) do
      newtc[k] = v
   end
   newtc.state = {} -- reset state for new context
   newtc.mode = tablex.deepcopy(torchcraft.mode)  -- reset mode for new context
   return newtc
end

function torchcraft:dmg_multiplier(wtype, usize)
    if wtype == self.dmgtypes.Concussive then
        if usize == self.unitsizes.Large then
            return 0.25
        elseif usize == self.unitsizes.Medium then
            return 0.5
        else
            return 1
        end
    elseif wtype == self.dmgtypes.Explosive then
        if usize == self.unitsizes.Small then
            return 0.5
        elseif usize == self.unitsizes.Medium then
            return 0.75
        else
            return 1
        end
    else
        return 1
    end
end

function torchcraft:_n_attacks(utype, air)
    if utype == self.unittypes.Protoss_Zealot then
        return 2 -- can sometimes be 1
    elseif utype == self.unittypes.Terran_Firebat then
        return 3 -- can sometimes be 2
    elseif utype == self.unittypes.Terran_Valkyrie then
        return 4
    elseif air and utype == self.unittypes.Terran_Goliath then
        return 2
    elseif air and utype == self.unittypes.Terran_Scout then
        return 2
    else
        return 1
    end
end

function torchcraft:get_weapon(source, dest)
    local wd = 0
    local weapontype = -1
    local range = 0
    local air = false
    if self.staticdata.isFlyer[dest.type] then
        wd = source.awattack -- n attacks and dmg multiplier accounted for
        weapontype = source.awtype
        range = source.awrange
        air = true
    else
        wd = source.gwattack -- n attacks and dmg multiplier accounted for
        weapontype = source.gwtype
        range = source.gwrange
    end
    return wd, weapontype, range, air
end

function torchcraft:compute_dmg(source, dest)
    local wd, weapontype, _, air = self:get_weapon(source, dest)
    if wd <= 0 then
        return 0, 0, 0
    end
    local wd = wd
    local shielddmg = 0
    if dest.shield > 0 then
        local tmp = (wd - self:_n_attacks(source, air) * dest.shieldArmor)
        if dest.shield >= tmp then
            return tmp, 0, tmp
        end
        shielddmg = dest.shield
        wd = wd - dest.shield
    end
    local hpdmg = self:dmg_multiplier(weapontype, dest.size)*wd
        - self:_n_attacks(source, air) * dest.armor
    return hpdmg + shielddmg, hpdmg, shielddmg
end

function torchcraft:translate(pos, vel, frames)
    if frames <= 0 then
        return {pos[1], pos[2]}
    end
    -- pos is in walktiles and vel is in pixels/frame
    local f = frames / self.xy_pixels_per_walktile
    return {pos[1] + torch.round(vel[1] * f),
            pos[2] + torch.round(vel[2] * f)}
end

function torchcraft:do_move(pos, vel, move, utype, frames)
    -- Here we do not change the velocity, only the position!
    local f = frames
    -- According to the following paper, it takes ~6 frames to start moving
    -- StarCraft Unit Motion: Analysis and Search Enhancements
    -- D Schneider, M Buro (AIIDE 2015)
    -- https://skatgame.net/mburo/ps/aiide15-motion.pdf
    -- We take this delay into account here (in the main branch) and not
    -- in the "if velocity == 0" branch because we suppose the effects
    -- of velocity are already accounted for, so we do not want to count
    -- a two move vectors (we consider it to be better to be wrong in
    -- direction than a little less wrong in direction but being wrong in
    -- magnitude too).
    f = f - 6
    if vel[1] == 0 and vel[2] == 0 then
        -- crudest way to take acceleration into account
        local accel = self.staticdata.acceleration[utype]
        if accel > 1 then
            f = f - math.ceil(accel/50) -- TODO check/validate experimentally
        end
    elseif (vel[1] * move[1] + vel[2] * move[2]) <= 0 then
        local turnradius = self.staticdata.turnRadius[utype]
        if turnradius > 1 then
            f = f - math.ceil(turnradius/40) -- TODO check/validate
        end
        -- TODO should be proportional to dot product...
    end
    local ts = self.staticdata.topSpeed[utype]
    return self:translate(pos, {move[1] * ts, move[2] * ts}, f)
end

function torchcraft:in_range(source, dest)
    -- Should I add some slack here? TODO
    local wd, _, range, _ = self:get_weapon(source, dest)
    if wd > 0 -- checks that source can fire on dest
        and utils.distance(source.position, dest.position) <= range then
        return true
    end
    return false
end

function torchcraft:apply_attack(source, dest, frames)
    -- in place modification of source and dest
    -- I'm not adding "min stop" frames from
    -- https://docs.google.com/spreadsheets/d/1bsvPvFil-kpvEUfSG74U3E5PLSTC02JxSkiR8QdLMuw/edit#gid=0
    -- Nor counting bullet fly times
    if source.gwcd - frames <= 0 -- source.gwcd == source.awcd
        and self:in_range(source, dest) then
        local _, hpdmg, shielddmg = self:compute_dmg(source, dest)
        source.gwcd = source.maxcd + source.gwcd - frames
        source.awcd = source.gwcd
        dest.hp = dest.hp - hpdmg
        dest.shield = dest.shield - shielddmg
    end
end

function torchcraft:fake_velocity(pos, npos, utype)
    -- just get direction right and magnitude not too wrong
    local vel = {npos[1] - pos[1], npos[2] - pos[2]}
    local ts = self.staticdata.topSpeed[utype]
    vel[1] = math.min(math.max(vel[1], -ts), ts)
    vel[2] = math.min(math.max(vel[2], -ts), ts)
    return vel
end

function torchcraft:apply_move(source, move, frames)
    -- in place modification of source
    local next_position = self:do_move(source.position, source.velocity,
        move, source.type, frames)
    local fake_velocity = self:fake_velocity(source.position,
        next_position, source.type)
    source.velocity = fake_velocity
    source.position = next_position
end

function torchcraft:imgMsgToImage(img_msg, img_mode)
  local img = nil
  if img_mode == "raw" then
    -- expecting "width,height,data"
    local it = string.gmatch(img_msg, "%d+,")
    local width = it()
    local height = it()
    width = tonumber(string.sub(width, 1, -2))
    height = tonumber(string.sub(height, 1, -2))
    -- remove data from img_msg
    local _, second_comma = string.find(img_msg, "%d+,%d+,", 1)
    img_msg = string.sub(img_msg, second_comma + 1)
    img = replayer.rawBitmapToTensor(img_msg, width, height)
  end
  return img
end

function torchcraft:is_unit_in_screen(source)
    local screen_x, screen_y = unpack(self.state.screen_position)
    local field_x, field_y = unpack(self.field_size)

    return ((source.pixel_x + (source.pixel_size_x / 2)>= screen_x) and
            (source.pixel_y + (source.pixel_size_y / 2) >= screen_y) and
            (source.pixel_x < (screen_x + field_x + (source.pixel_size_x / 2))) and
            -- bounding box a bit too precise, let's be conservative
            (source.pixel_y < (screen_y + field_y + (source.pixel_size_y / 3))))
end

function torchcraft:draw_entity(img, pos_x, pos_y, size_x, size_y, rgb)
    pos_x = pos_x / 640 * img:size()[3]
    pos_y = pos_y / 480 * img:size()[2]

    local narrow_width_start = math.max(1,
                                        math.floor(pos_x - size_x / 2))
    local narrow_width_end = math.min(img:size()[3],
                                      math.floor(pos_x + size_x / 2))
    local narrow_height_start = math.max(1,
                                         math.floor(pos_y - size_y / 2))
    local narrow_height_end = math.min(img:size()[2],
                                       math.floor(pos_y + size_y / 2))
    -- Hack to fix crash were unit is on top left corner of FoV
    if narrow_width_end <= 0 then
        narrow_width_end = narrow_width_start
    end
    local rect = img:narrow(3, narrow_width_start,
                            narrow_width_end - narrow_width_start + 1)
    if narrow_height_end <= 0 then
        narrow_height_end = narrow_height_start
    end
    local rect = rect:narrow(2, narrow_height_start,
                             narrow_height_end - narrow_height_start + 1)
    rect[1]:fill(rgb[1])
    rect[2]:fill(rgb[2])
    rect[3]:fill(rgb[3])
    return img
end

function torchcraft:get_unit_screen_pos(ut)
    -- returns nil otherwise
    if self:is_unit_in_screen(ut) then
        local screen_x, screen_y = unpack(self.state.screen_position)
        return ut.pixel_x - screen_x, ut.pixel_y - screen_y
    end
end

function torchcraft:get_layer(layer_type, also_enemy)
    -- add background
    local img = torch.ByteTensor(self.state.image:size()):fill(0)
    -- add units
    local pos_x, pos_y, color
    local t = self.state.units_myself
    if also_enemy then
        t = tablex.merge(t, self.state.units_enemy, true)
    end
    if layer_type == "visibility" then
        for y, xs in ipairs(self.state.visibility) do
            for x, value in ipairs(xs) do
                pos_x = (x - 1) * 32
                pos_y = (y - 1) * 32
                color = utils.visibility_color_table[value]
                img = self:draw_entity(img, pos_x, pos_y,
                                       32, 32, color)
            end
        end
    elseif layer_type == "minerals" then
        t = self.state.units_neutral
        for uid, ut in pairs(t) do
            if self:is_unit_in_screen(ut) and self:is_mineral_field(ut.type) then
                pos_x, pos_y = self:get_unit_screen_pos(ut)
                color = utils.get_health_color(ut.resource, 1500)
                img = self:draw_entity(img, pos_x, pos_y,
                                       ut.pixel_size_x, ut.pixel_size_y, color)
            end
        end
    elseif layer_type == "gas" then
        t = tablex.merge(t, self.state.units_neutral, true);
        for uid, ut in pairs(t) do
            if self:is_unit_in_screen(ut) and self:is_gas_geyser(ut.type) then
                pos_x, pos_y = self:get_unit_screen_pos(ut)
                color = utils.get_health_color(ut.resource, 2500)
                img = self:draw_entity(img, pos_x, pos_y,
                                       ut.pixel_size_x, ut.pixel_size_y, color)
            end
        end
    else
        for uid, ut in pairs(t) do
            if self:is_unit_in_screen(ut) then
                pos_x, pos_y = self:get_unit_screen_pos(ut)
                if layer_type == "hp" and ut.max_hp > 0 then
                    color = utils.get_health_color(ut.hp, ut.max_hp)
                elseif layer_type == "type" then
                    color = utils.html_color_table[ut.type]
                elseif layer_type == "player" then
                    color = utils.players_color_table[ut.playerId]
                elseif layer_type == "shield" and ut.shield > 0 then
                    color = utils.get_health_color(ut.shield, ut.max_shield)
                elseif layer_type == "energy" then
                    color = utils.get_health_color(ut.energy, 250)
                end
                if color then
                    img = self:draw_entity(img, pos_x, pos_y,
                                           ut.pixel_size_x, ut.pixel_size_y,
                                           color)
                end
            end
        end
    end
    return img
end

function torchcraft:draw_value(img, pos_x, pos_y, size_x, size_y, value)
    local narrow_width_start = math.max(1,
                                        math.floor(pos_x - size_x / 2))
    local narrow_width_end = math.min(self.field_size[1],
                                      math.floor(pos_x + size_x / 2))
    local narrow_height_start = math.max(1,
                                         math.floor(pos_y - size_y / 2))
    local narrow_height_end = math.min(self.field_size[2],
                                       math.floor(pos_y + size_y / 2))
    -- Hack to fix crash were unit is on top left corner of FoV
    if narrow_width_end <= 0 then
        narrow_width_end = narrow_width_start
    end
    local rect = img:narrow(2, narrow_width_start,
                            narrow_width_end - narrow_width_start + 1)
    if narrow_height_end <= 0 then
        narrow_height_end = narrow_height_start
    end
    local rect = rect:narrow(1, narrow_height_start,
                             narrow_height_end - narrow_height_start + 1)
    rect:fill(value)
    return img
end

function torchcraft:get_feature(feature, also_enemy)
--returns 2D imagelike tensor of feature
--type defaults to ByteTensor

-- :TODO: Maybe make categorical features return a 3D tensor of one-hots
--        Or just render them to colors with draw value to increase spacing

    local img  = torch.ShortTensor(self.field_size[2],self.field_size[1]):fill(0)
    local pos_x, pos_y
    local t

    if feature == "visibility" then
        for y, xs in ipairs(self.state.visibility) do
            for x, value in ipairs(xs) do
                pos_x = (x - 1) * 32
                pos_y = (y - 1) * 32
                img = self:draw_value(img, pos_x, pos_y,
                                       32, 32, value)
            end
        end
    elseif feature == "minerals" then
        t = self.state.units_neutral
        for uid, ut in pairs(t) do
            if self:is_unit_in_screen(ut) and self:is_mineral_field(ut.type) then
                pos_x, pos_y = self:get_unit_screen_pos(ut)
                img = self:draw_value(img, pos_x, pos_y,
                                    ut.pixel_size_x, ut.pixel_size_y, ut.resource)
            end
        end
        return img
    elseif feature == "gas" then

        t = tablex.merge(self.state.units_myself, self.state.units_neutral, true)

        for uid, ut in pairs(t) do
            if self:is_unit_in_screen(ut) and self:is_gas_geyser(ut.type) then
                pos_x, pos_y = self:get_unit_screen_pos(ut)
                img = self:draw_value(img, pos_x, pos_y,
                                    ut.pixel_size_x, ut.pixel_size_y, ut.resource)
            end
        end
        return img
    else
        t = self.state.units_myself
        if also_enemy then
            t = tablex.merge(t, self.state.units_enemy, true)
        end
        -- adding one to these so they aren't 0 indexed, otherwise
        -- they can become part of the background
        if feature == "type" or feature == "playerId" then
            for uid, ut in pairs(t) do
                if self:is_unit_in_screen(ut) then
                    pos_x, pos_y = self:get_unit_screen_pos(ut)
                    if pos_x then
                        img = self:draw_value(img, pos_x, pos_y,
                            ut.pixel_size_x, ut.pixel_size_y, (ut[feature] + 1))
                    end
                end
            end
        else
            for uid, ut in pairs(t) do
                if self:is_unit_in_screen(ut) then
                    pos_x, pos_y = self:get_unit_screen_pos(ut)
                    if pos_x then
                        img = self:draw_value(img, pos_x, pos_y,
                            ut.pixel_size_x, ut.pixel_size_y, ut[feature])
                    end
                end
            end
        end
    end

    return img
end

return torchcraft
