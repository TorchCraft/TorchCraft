--[[
   Copyright (c) 2015-present, Facebook, Inc.
   All rights reserved.
   This source code is licensed under the BSD-style license found in the
   LICENSE file in the root directory of this source tree. An additional grant
   of patent rights can be found in the PATENTS file in the same directory.
]]--

-- attacks the closest units, th simple_exe.lua [-t $hostname] [-p $port]
local DEBUG = 0 -- can take values 0, 1, 2 (from no output to most verbose)
require 'torch'
torch.setdefaulttensortype('torch.FloatTensor')
require 'sys'
local lapp = require 'pl.lapp'
local args = lapp [[
Baselines for Starcraft
    -t,--hostname       (default "")    Give hostname / ip pointing to VM
    -p,--port           (default 11111) Port for torchcraft. Do 0 to grab vm from cloud
]]

local skip_frames = 7
local port = args.port
local hostname = args.hostname or ""
print("hostname:", hostname)
print("port:", port)

require 'progressbar'
local progress = ProgressBar(-1)
progress.mutex = {lock = function() end, unlock = function() end}
progress:singleThreaded()

local tc = require 'torchcraft'
tc.DEBUG = DEBUG
local utils = require 'torchcraft.utils'

local function get_closest(position, unitsTable)
    local min_d = 1E30
    local closest_uid = nil
    for uid, ut in pairs(unitsTable) do
        local tmp_d = utils.distance(position, ut['position'])
        if tmp_d < min_d then
            min_d = tmp_d
            closest_uid = uid
        end
    end
    return closest_uid
end

local battles_won = 0
local battles_game = 0
local total_battles = 0

local frames_in_battle = 1
local nloop = 1

-- no ',' accepted in map names!
-- All paths must be relative to C:/StarCraft
local maps = {'Maps/BroodWar/micro/dragoons_zealots.scm',
              'Maps/BroodWar/micro/m5v5_c_far.scm'}

tc.mode.micro_battles = true
tc.mode.replay = false
-- This overwrites whatever is in bwapi.ini
tc.initial_map = maps[2]

-- connects to the StarCraft launched by BWEnv.exe
tc:init(hostname, port)
local update = tc:connect(port)

if DEBUG > 1 then
    print('Received init: ', update)
end

-- first message to BWAPI's side is setting up variables
local setup = {
    tc.command(tc.set_speed, 0), tc.command(tc.set_gui, 1),
    tc.command(tc.set_frameskip, 400),
    tc.command(tc.set_cmd_optim, 1),
}
tc:send({table.concat(setup, ':')})


local nrestarts = -1

while total_battles < 40 do

    print("")
    print("CTRL-C to stop")
    print("")

    battles_won = 0
    battles_game = 0
    nrestarts = nrestarts + 1

    -- initialize TorchCraft's state
    tc:set_variables()

    local tm = torch.Timer()
    while not tc.state.game_ended do
        local done = false

        update = tc:receive()
        if DEBUG > 1 then
            print('Received update: ', update)
        end

        nloop = nloop + 1
        local actions = {}
        if tc.state.game_ended then
            done = true
        elseif tc.state.battle_just_ended then
            if DEBUG > 0 then
                print("BATTLE ENDED")
            end
            if tc.state.battle_won then -- we won (in micro battles)
                battles_won = battles_won + 1
            end
            battles_game = battles_game + 1
            total_battles = total_battles + 1
            frames_in_battle = 0
            if battles_game >= 10 then -- this is an example
                done = true
            end
        elseif tc.state.waiting_for_restart then
            -- a battle finished, waiting for the next one to start!
            if DEBUG > 0 then
                print("WAITING FOR RESTART")
            end
        else
            if tc.state.battle_frame_count % skip_frames == 0 then
                for uid, ut in pairs(tc.state.units_myself) do
                    local target = get_closest(ut.position,
                                               tc.state.units_enemy)
                    if target ~= nil then
                        table.insert(actions,
                            tc.command(tc.command_unit_protected, uid,
                                        tc.cmd.Attack_Unit, target))
                    end
                end
                if frames_in_battle > 2*60*24 then -- quit after ~ 2 hours
                    actions = {tc.command(tc.quit)}
                    nrestarts = nrestarts + 1
                end
                progress:pop()
            end
        end

        progress:add('Loop', nloop, '%5d')
        progress:add('FPS', 1 / tm:time().real, '%5d')
        progress:add('WR', battles_won / (battles_game+1E-6), '%1.3f')
        progress:add('#Wins', battles_won, '%4d')
        progress:add('#Bttls', battles_game, '%4d')
        progress:push()
        tm:reset()
        if done then break end -- So the last progress bar update happens

        if DEBUG > 1 then
            print("")
            print("Sending actions:")
            print(actions)
        end
        tc:send({table.concat(actions, ':')})
    end

    local m = maps[(nrestarts % #maps) + 1]
    local setup = {
        tc.command(tc.set_map, m),
        tc.command(tc.restart),
        tc.command(tc.set_speed, 0), tc.command(tc.set_gui, 1),
        tc.command(tc.set_frameskip, 400),
        tc.command(tc.set_cmd_optim, 1),
    }
    print("")
    print("Current map name: ", tc.state.map_name)
    print("Sent map name: ", m)

    tc:send({table.concat(setup, ':')})
    os.execute("sleep 0.5")
    print("")
    progress:reset()
    print("")
    collectgarbage()
    collectgarbage()
end

tc:receive()
tc:send({table.concat({tc.command(tc.exit_process)})})
print("")
