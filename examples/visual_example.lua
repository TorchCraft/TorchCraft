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
require 'image'
local stringx = require 'pl.stringx'
local tablex = require 'pl.tablex'
local lapp = require 'pl.lapp'

local args = lapp [[
    Baselines for Starcraft
    -t,--hostname       (default "")    Give hostname / ip pointing to VM
    -p,--port           (default 11111) Port for torchcraft. Do 0 to grab vm from cloud
    -s,--skip_frames    (default 30)     Number of frames to skip per calculation
    -m,--micro_game     (default true)  Set to false to run on a normal map (test resource layers)
    ]]

local skip_frames = args.skip_frames
print("skip frames:", skip_frames)
local max_battles = 10
local port = args.port
local hostname = args.hostname or ""
local micro_game = args.micro_game
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

tc.initial_map = 'Maps/BroodWar/micro/m5v5_c_far.scm'

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

os.execute('mkdir -p images')

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
            local hp_layer         = tc:get_layer("hp",         true)
            local type_layer       = tc:get_layer("type",       true)
            local player_layer     = tc:get_layer("player",     true)
            local visibility_layer = tc:get_layer("visibility", true)
            local minerals_layer   = tc:get_layer("minerals",   true)
            local gas_layer        = tc:get_layer("gas",        true)
            local energy_layer     = tc:get_layer("energy",     true)
            local shield_layer     = tc:get_layer("shield",     true)

            if save_continuous then
                cstr = string.format("%05d", img_counter)
                img_counter = img_counter + 1
            end

            image.save("images/img_" .. cstr .. ".ppm", tc.state.image)
            image.save("images/layer_health_" .. cstr .. ".ppm", hp_layer)
            image.save("images/layer_type_" .. cstr .. ".ppm", type_layer)
            image.save("images/layer_player_" .. cstr .. ".ppm", player_layer)
            image.save("images/layer_visibility_" .. cstr .. ".ppm", visibility_layer)
            image.save("images/layer_shield_"   .. cstr .. ".ppm", shield_layer)
            image.save("images/layer_energy_"   .. cstr .. ".ppm", energy_layer)
            if not micro_game then
                image.save("images/layer_minerals_" .. cstr .. ".ppm", minerals_layer)
                image.save("images/layer_gas_"      .. cstr .. ".ppm", gas_layer)
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

        if frames_in_battle > 2*60*24 then
            actions = {tc.command(tc.quit)}
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
