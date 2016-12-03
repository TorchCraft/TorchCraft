--[[
   Copyright (c) 2015-present, Facebook, Inc.
   All rights reserved.
   This source code is licensed under the BSD-style license found in the
   LICENSE file in the root directory of this source tree. An additional grant
   of patent rights can be found in the PATENTS file in the same directory.
]]--

-- This works with AIModule / DLL BWEnv
-- heuristic can take value in:
-- * wc: weakest and closest
-- * w: weakest
-- * c: closest
-- * noop: litteraly send only noops
-- * rand: select a target at random
-- * rand_nc: rand but without target changes if current target is valid
-- * rand_move: random moves + target
-- * def: weakest in range, with some slack (8 build tiles), defensive
-- * defnok: same as above but without overkill
-- * def_nc: def but without target changes if current target is valid
-- * defnok_nc: defnok but without target changes if current target is valid
local DEBUG = 0 -- can take values 0, 1, 2 (from no output to most verbose)
require 'torch'
torch.setdefaulttensortype('torch.FloatTensor')
require 'sys'
local tablex = require 'pl.tablex'
local lapp = require 'pl.lapp'
local args = lapp [[
Baselines for Starcraft
    -h,--heuristic      (default "wc")  Check comments for list of heuristics
    -t,--hostname       (default "")    Give hostname / ip pointing to VM
    -p,--port           (default 11111) Port for torchcraft. Do 0 to grab vm from cloud
    -s,--skip_frames    (default 9)     Number of frames to skip per calculation
    --one_loop_only                     Only runs for one loop
    -o,--output         (default "")    Where to write out a log
    --max_battles       (default 200)   Number of max battles before quitting the current micro map
    -m,--total_battles  (default 1000)  Total number of battles to run for
    -c,--config         (default "")    If using cloud, we can upload a config file
    -l,--log_actions                    Log actions on starcraft VM
    --record_to         (default "")    Record battles in this directory
]]

local heuristic = args.heuristic
print("using heuristic:", heuristic)
local skip_frames = args.skip_frames
print("skip frames:", skip_frames)
local one_loop_only = args.one_loop_only
local log_fname = args.output
local max_battles = args.max_battles
local port = args.port
local hostname = args.hostname or ""
print("hostname:", hostname)
print("max battles:", max_battles)

require 'progressbar'
local progress = ProgressBar(-1)
progress.mutex = {lock = function() end, unlock = function() end} -- hack
progress:singleThreaded()
local tc = require 'torchcraft'
local utils = require 'torchcraft.utils'
local replayer = require 'torchcraft.replayer'
local heuristics = require 'targeting_heuristics'
local isFlyer = tc.staticdata.isFlyer
tc.DEBUG = DEBUG

local battles_won = 0
local battles_game = 0
local wf
if log_fname ~= "" then
    wf = io.open(log_fname, 'w')
    local pretty = require 'pl.pretty'
    wf:write('baseline attack\n')
    wf:write('args: ' .. pretty.write(arg) .. '\n')
end

local record_mode = (args.record_to ~= "")
local rep = nil

local first_loop = true
local total_battles = 0
while (first_loop or not one_loop_only)
    and total_battles < args.total_battles do
    local frames_in_battle = 1
    local last_frame_from_bwapi = 0
    local nloop = 1
    battles_won = 0
    battles_game = 0

    -- connects to the StarCraft running with BWEnv
    tc.mode.micro_battles = true
    tc.mode.replay = false

    tc:init(hostname, port)
    -- first message from the BWAPI side is setting up variables
    local update = tc:connect(port)
    if DEBUG > 1 then
        print('Received init: ', update)
    end

    tc:send({table.concat({
        tc.command(tc.set_speed, 0), tc.command(tc.set_gui, 1),
        tc.command(tc.set_frameskip, 400),
        tc.command(tc.set_log, args.log_actions and 1 or 0),
        tc.command(tc.set_cmd_optim, 1),
        tc.command(tc.set_combine_frames, skip_frames)
    }, ':')})

    print("\nMap name: ", tc.state.map_name)
    if log_fname ~= "" then
        wf:write("Map name: " .. tc.state.map_name .. '\n')
    end

    local tm = torch.Timer()
    while not tc.state.game_ended do
        progress:add('Loop', nloop, '%5d')
        progress:add('FPS', 1 / tm:time().real, '%5d')
        progress:add('WR', battles_won / (battles_game+1E-6), '%1.3f')
        progress:add('#Wins', battles_won, '%4d')
        progress:add('#Bttls', battles_game, '%4d')
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
            if battles_game >= max_battles then -- this is an example
                actions = {tc.command(tc.quit)}
            end
            if log_fname ~= "" then
                wf:write(string.format("Won: %i\n",
                          tc.state.battle_won and 1 or 0))
                wf:flush()
            end
            if record_mode and rep ~= nil then
                rep:save(args.record_to .. "/" .. total_battles .. ".rep")
                rep = nil
            end
        elseif tc.state.waiting_for_restart then
            -- a battle finished, waiting for the next one to start!
            if DEBUG > 0 then
                print("WAITING FOR RESTART")
            end
        else
            assert(tc.state.battle_frame_count % skip_frames == 0,
                    "Frame count: " .. tc.state.battle_frame_count)

            if record_mode then
                if rep == nil then
                    rep = replayer.newReplayer()
                    rep:setMap(tc.state.map_data)
                end
                rep:push(replayer.frameFromTable({
                    state = {[0] = tc.state.units_myself,
                             [1] = tc.state.units_enemy},
                    actions = {},
                    reward = 0,
                    is_terminal = false
                }))
            end

            local targets = nil
            if heuristic == 'defnok' then
                targets = get_weakest_in_range_slack_no_overkill(
                              tc.state.units_enemy,
                              tc.state.units_myself,
                              true)
            elseif heuristic == 'defnok_nc' then
                targets = get_weakest_in_range_slack_no_overkill(
                              tc.state.units_enemy,
                              tc.state.units_myself,
                              false)
            end
            for uid, ut in pairs(tc.state.units_myself) do
                local target, position
                if heuristic == 'noop' then
                    target = nil
                elseif heuristic == 'def' then
                    target = heuristics.get_weakest_in_range_slack(
                                 tc.state.units_enemy,
                                 ut)
                elseif heuristic == 'def_nc' then
                    target = heuristics.dont_change_target(heuristics
                        .get_weakest_in_range_slack(
                             tc.state.units_enemy,
                             ut), ut, tc.state.units_enemy)
                elseif heuristic == 'rand' then
                    target = heuristics.get_random(tc.state.units_enemy)
                elseif heuristic == 'rand_nc' then
                    target, position = heuristics.dont_change_target(
                                 heuristics.get_random(tc.state.units_enemy),
                                 ut, tc.state.units_enemy)
                elseif heuristic == 'rand_move' then
                    target, position = heuristics.get_random_action(
                                           ut.position, tc.state.units_enemy)
                elseif heuristic == 'w' then
                    target = utils.get_weakest(tc.state.units_enemy)
                elseif heuristic == 'c' then
                    target = utils.get_closest(ut.position,
                                               tc.state.units_enemy)
                elseif heuristic == 'wc' then
                    target = heuristics.get_weakest_and_closest(
                                 tc.state.units_enemy,
                                 tc.state.units_myself)
                elseif heuristic == 'defnok' then
                    target = heuristics.targets[uid]
                elseif heuristic == 'moveleft' then
                    position = {ut.position[1] - 10, ut.position[2]}
                elseif heuristic == 'moveright' then
                    position = {ut.position[1] + 10, ut.position[2]}
                end
                if target ~= nil then
                    table.insert(actions,
                        tc.command(tc.command_unit_protected, uid,
                                    tc.cmd.Attack_Unit, target))
                elseif position ~= nil then
                    table.insert(actions,
                        tc.command(tc.command_unit_protected, uid,
                                   tc.cmd.Move, -1,
                                   position[1], position[2]))
                end
            end

            if tc.state.frame_from_bwapi > last_frame_from_bwapi then
                frames_in_battle = frames_in_battle + 1
                last_frame_from_bwapi = tc.state.frame_from_bwapi
            end
            if frames_in_battle > 2*60*24 then
                actions = {tc.command(tc.quit)}
            end
            -- print('Sent actions')
            progress:pop()
        end
        if total_battles >= args.total_battles then
            actions = {tc.command(tc.quit)}
        end
        if DEBUG > 1 then
            print("")
            print("Sending actions:")
            print(actions)
        end
        tc:send({table.concat(actions, ':')})
    end
    print("")
    tc:close()
    os.execute("sleep 0.5")
    progress:reset()
    print("")
    collectgarbage()
    collectgarbage()
    first_loop = false
end
print("")
