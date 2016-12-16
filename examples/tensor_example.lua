--[[
   Copyright (c) 2015-present, Facebook, Inc.
   All rights reserved.
   This source code is licensed under the BSD-style license found in the
   LICENSE file in the root directory of this source tree. An additional grant
   of patent rights can be found in the PATENTS file in the same directory.
]]--

require 'torch'
torch.setdefaulttensortype('torch.FloatTensor')
require 'sys'
require 'gnuplot'
local stringx = require 'pl.stringx'
local tablex = require 'pl.tablex'
local lapp = require 'pl.lapp'

local args = lapp [[
    Baselines for Starcraft
    -t,--hostname       (default "")    Give hostname / ip pointing to VM
    -p,--port           (default 11111) Port for torchcraft. Do 0 to grab vm from cloud
    -s,--skip_frames    (default 240)     Number of frames to skip per calculation
    -m,--micro_game     (default true)  Set to false to run on a normal map (test resource layers)
    -f,--feature        (default hp) select what feature to view
    ]]

local skip_frames = args.skip_frames
print("skip frames:", skip_frames)
local max_battles = 10
local port = args.port
local hostname = args.hostname or ""
local micro_game = args.micro_game
local test_feature = args.feature
print("hostname:", hostname)
print("max battles:", max_battles)
print("micro_game:", micro_game)

require 'progressbar'
local progress = ProgressBar(-1)
progress.mutex = {lock = function() end, unlock = function() end}
progress:singleThreaded()

local tc = require 'torchcraft'
local utils = require 'torchcraft.utils'
local replayer = require 'torchcraft.replayer'
local isFlyer = tc.staticdata.isFlyer

local available_actions = {
    tc.usercmd.Move_Screen_Up,
    tc.usercmd.Move_Screen_Down,
    tc.usercmd.Move_Screen_Left,
    tc.usercmd.Move_Screen_Right
}
-- ~~~~~~~~~~~~ INITIALIZATION ~~~~~~~~~~~~~

local battles_game = 0
local frames_in_battle = 1
local last_frame_from_bwapi = 0
local nloop = 1

if micro_game then
    tc.initial_map = 'Maps/Broodwar/dragoons_zealots.scm'
else
    tc.initial_map = 'Maps/ICCup/ICCup Python 1.3.scx'
end

tc.window_pos = {200, 200}
tc.window_size = {320, 240}
tc.mode.micro_battles = micro_game
tc.mode.replay = false

tc:init(hostname, port)
local update = tc:connect(port)

-- first message from the BWAPI side is setting up variables
tc:set_variables()

tc:send({table.concat({
                 tc.command(tc.set_speed, 15), -- 13 is max speed without frame not being updated
                 tc.command(tc.set_gui, 1),
                 tc.command(tc.set_frameskip, 5),
                 tc.command(tc.set_cmd_optim, 1),
                 tc.command(tc.set_combine_frames, skip_frames)
                      }, ':')})

print("\nMap name: ", tc.state.map_name)

-- ~~~~~~~~~~~~~~ LOOP ~~~~~~~~~~~~~~~

local save_continuous = true
local cstr = ""
local img_counter = 0

local tensor_to_string = torch.Tensor.__tostring__

local tm = torch.Timer()
while not tc.state.game_ended do
    -- progress bar
    progress:add('Loop', nloop, '%7d')
    progress:add('FPS', 1 / tm:time().real, '%7d')
    if micro_game then
        progress:add('#Bttls', battles_game, '%7d')
    end
    progress:push()
    tm:reset()

    -- update turn data
    update = tc:receive()
    nloop = nloop + 1
    local actions = {}

    if tc.state.game_ended then
        break

    elseif microgame and tc.state.battle_just_ended then
        battles_game = battles_game + 1
        frames_in_battle = 0
        if battles_game >= max_battles then -- this is an example
            actions = {tc.command(tc.quit)}
        end

    elseif microgame and tc.state.waiting_for_restart then
        -- do nothing

    else
        if tc.state.image then

            -- Uncomment the following to check the changing screen position
            -- print(tc.state.screen_position)

            -- Uncomment the following to list units (and whether they are in the
            -- screen or not)
            -- for uid, ut in pairs(tc.state.units_myself) do
            --     print("Unit " .. uid .. " in screen: " .. tostring(tc:is_unit_in_screen(ut)))
            -- end

            local hp_tensor         = tc:get_feature("hp",         true)
            local type_tensor       = tc:get_feature("type",       true)
            local player_tensor     = tc:get_feature("playerId",   true)
            local visibility_tensor = tc:get_feature("visibility", true)
            local minerals_tensor   = tc:get_feature("minerals",   true)
            local gas_tensor        = tc:get_feature("gas",        true)
            local energy_tensor     = tc:get_feature("energy",     true)
            local shield_tensor     = tc:get_feature("shield",     true)


            if save_continuous then
                cstr = string.format("%05d", img_counter)
                img_counter = img_counter + 1
            end

            --                     #justvimthings V
            if     test_feature == "hp" then
                gnuplot.imagesc(hp_tensor         ,'color')
            elseif test_feature == "type" then
                gnuplot.imagesc(type_tensor       ,'color')  
            elseif test_feature == "player" then
                gnuplot.imagesc(player_tensor     ,'color')
            elseif test_feature == "visibility" then
                gnuplot.imagesc(visibility_tensor ,'color')
            elseif test_feature == "minerals" then
                gnuplot.imagesc(minerals_tensor   ,'color')
            elseif test_feature == "gas" then
                gnuplot.imagesc(gas_tensor        ,'color')
            elseif test_feature == "energy" then
                gnuplot.imagesc(energy_tensor     ,'color')
            elseif test_feature == "shield" then
                gnuplot.imagesc(shield_tensor     ,'color')
            end
            
        end

        -- Choose random screen action
        act = available_actions[math.random(#available_actions)]
        -- table.insert(actions, tc.command(tc.command_user, act, 10))
        table.insert(actions, tc.command(tc.noop))

        if tc.state.frame_from_bwapi > last_frame_from_bwapi then
            frames_in_battle = frames_in_battle + 1
            last_frame_from_bwapi = tc.state.frame_from_bwapi
        end

        progress:pop()
    end

    table.insert(actions,
                 tc.command(tc.request_image))

    tc:send({table.concat(actions, ':')})
end

tc:send({table.concat({
    tc.command(tc.exit_process)
}, ':')})

os.execute("sleep 0.5")
print("")
print()
collectgarbage()
collectgarbage()
