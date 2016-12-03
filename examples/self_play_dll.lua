--[[
   Copyright (c) 2015-present, Facebook, Inc.
   All rights reserved.
   This source code is licensed under the BSD-style license found in the
   LICENSE file in the root directory of this source tree. An additional grant
   of patent rights can be found in the PATENTS file in the same directory.
]]--

-- Self play doesn't work with exe, you must launch 2 DLLs! 
--   Copy TorchCraft/BWEnv/bin/* to C:/StarCraft
--   Right click self_play.bat -> Run as Administrator
--   Linux: ./launch_x_sc.sh 2 5
--   Launch two instances of self_play_dll.lua:
--     th self_play_dll.lua --host
--     th self_play_dll.lua
--
-- attacks the closest units
local DEBUG = 0 -- can take values 0, 1, 2 (from no output to most verbose)
require 'torch'
torch.setdefaulttensortype('torch.FloatTensor')
require 'sys'
local lapp = require 'pl.lapp'
local args = lapp [[
Baselines for Starcraft
    -t,--hostname       (default "")    Give hostname / ip pointing to VM
    -h,--heuristic      (default "wc")  Check comments for list of heuristics
    -p,--port           (default 0)     Port for torchcraft.
    --host                              Host of self_play game
]]

local skip_frames = 9
local port = args.port
if port == 0 then port = args.host and 11111 or 11112 end
local hostname = args.hostname or ""
print("hostname:", hostname)
print("port:", port)

require 'progressbar'
local progress = ProgressBar(-1)
progress.mutex = {lock = function() end, unlock = function() end} -- hack
progress:singleThreaded()

local tc = require 'torchcraft'
tc.DEBUG = DEBUG
local utils = require 'torchcraft.utils'

function wait_for_host()
    if not args.host then sys.sleep(10) end
end

local battles_won = 0
local battles_game = 0
local total_battles = 0

local frames_in_battle = 1
local nloop = 1

-- no ',' accepted in map names!
-- All paths must be relative to C:/StarCraft
local maps = {'Maps/BroodWar/micro/sp_dragoons_zealots.scm',
              'Maps/BroodWar/micro/sp_m5v5_c_far.scm'}

tc.mode.micro_battles = true
tc.mode.replay = false

local nrestarts = 0

tc:init(hostname, port)
local update = tc:connect(port)
if DEBUG > 1 then
    print('Received init: ', update)
end
wait_for_host()
local set_map = {
    tc.command(tc.set_map, maps[(nrestarts % #maps) + 1]),
    tc.command(tc.set_multi, 1),
    tc.command(tc.quit),
}
tc:send({table.concat(set_map, ':')})
tc:close()

while total_battles < 40 do
    print("CTRL-C to stop")
    battles_won = 0
    battles_game = 0
    nrestarts = nrestarts + 1

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

    local tm = torch.Timer()
    while not tc.state.game_ended do
        progress:add('Loop', nloop, '%5d')
        progress:add('FPS', 1 / tm:time().real, '%5d')
        progress:add('WR', battles_won / (battles_game+1E-6), '%1.3f')
        progress:add('#Wins', battles_won, '%4d')
        progress:add('#Bttls', battles_game, '%4d')
        progress:add('Tot Bttls', total_battles, '%4d')
        progress:push()
        tm:reset()

        update = tc:receive()
        if DEBUG > 1 then
            print('Received update: ', update)
        end

        nloop = nloop + 1
        local actions = {}
        if tc.state.game_ended then
            break
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
            if battles_game >= 10 then
                wait_for_host()
                actions = {
                    tc.command(tc.set_map, maps[(nrestarts % #maps) + 1]),
                    tc.command(tc.quit),
                }
            end
        elseif tc.state.waiting_for_restart then
            -- a battle finished, waiting for the next one to start!
            if DEBUG > 0 then
                print("WAITING FOR RESTART")
            end
        else
            if tc.state.battle_frame_count % skip_frames == 0 then
                for uid, ut in pairs(tc.state.units_myself) do
                    local target = utils.get_closest(ut.position,
                                                     tc.state.units_enemy)
                    if target ~= nil then
                        table.insert(actions,
                        tc.command(tc.command_unit_protected, uid,
                        tc.cmd.Attack_Unit, target))
                    end
                end
                if frames_in_battle > 2*60*24 then -- quit after ~ 2 hours
                    wait_for_host()
                    actions = {tc.command(tc.quit)}
                    nrestarts = nrestarts + 1
                end
                progress:pop()
            end
        end

        if DEBUG > 1 then
            print("")
            print("Sending actions:")
            print(actions)
        end
        tc:send({table.concat(actions, ':')})
    end

	tc:close()
	sys.sleep(0.5)
    print()
    progress:reset()
    print()
    collectgarbage()
    collectgarbage()
end
print()

