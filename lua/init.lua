local torchcraft = require 'torchcraft._env'
local utils = require 'torchcraft.utils'
local replayer = require 'torchcraft.replayer'
local tablex = require 'pl.tablex'
local image = require 'image'
local lib = require 'torchcraft.tc_lib'
for k,v in pairs(lib) do
    torchcraft[k] = v
end

-- Pull constants into module namespace
for k,v in pairs(lib.const) do
    torchcraft[k] = v
end
for k,v in pairs(lib.const.commands) do
    torchcraft[k] = v
end

-- Shortcuts
torchcraft.cmd = torchcraft.unitcommandtypes
torchcraft.usercmd = torchcraft.usercommandtypes

assert(torchcraft.total_price.mineral[torchcraft.unittypes.Zerg_Guardian]
    == torchcraft.staticdata.mineralPrice[torchcraft.unittypes.Zerg_Mutalisk]
       + torchcraft.staticdata.mineralPrice[torchcraft.unittypes.Zerg_Guardian])
assert(torchcraft.total_price.mineral[torchcraft.unittypes.Protoss_Dark_Archon]
    == 2 * torchcraft.staticdata.mineralPrice[torchcraft.unittypes.Protoss_Dark_Templar])
assert(torchcraft.total_price.gas[torchcraft.unittypes.Terran_Science_Vessel]
    == 225)


torchcraft.hostname = nil
torchcraft.client = torchcraft.Client()
torchcraft.state = torchcraft.client.state
torchcraft.DEBUG = 0
torchcraft.initial_map = nil
torchcraft.window_size = nil
torchcraft.window_pos = nil
torchcraft.micro_battles = false
torchcraft.only_consider_types = {}
torchcraft.field_size = {640, 370}   -- size of the field view in pixels (approximately)
--[[
    state will get its content updated from bwapi, it will have
    * ground_height_data  : [torch.ByteTensor] 2D. 255 (-1) where not walkable
    * walkable_data       : [torch.ByteTensor] 2D.
    * buildable_data      : [torch.ByteTensor] 2D.
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
        local arpstring = 'arp -a -i vmnet8 | grep -v "ff:ff:ff:ff:ff:ff" | grep -v incomplete | '
                          .. 'tail -1 | cut -f2 -d" " | tr -d "()"'
        print("executing: " .. arpstring)
        self.hostname = sys.fexecute(arpstring)
    else
        self.hostname = hostname
    end
    self.port = port ~= nil and port or (os.getenv('TorchCraft_PORT') or 11111)
    print('host: ' .. self.hostname .. ':' .. self.port)
end

function torchcraft:connect(port, timeoutMs)
    -- connect() should be called at the beginning of every game
    if self.hostname == nil or self.hostname == '' then
        self:init(nil, port)
    end
    -- timeout for send / receive operations
    timeoutMs = timeoutMs or -1
    -- initialize socket connection, use the specified timeouts
    self.client:connect(self.hostname, self.port, timeoutMs)

    self.state = self.client.state

    local ok, setup = pcall(function() return self.client:init({
        initial_map = self.initial_map,
        window_size = self.window_size,
        window_pos = self.window_pos,
        micro_battles = self.micro_battles,
        only_consider_types = self.only_consider_types,
    }) end)
    -- reset client's connection to leave TC object in a consistent state
    if not ok then
        local ok, err = pcall(function() self.client:close() end)
        if not ok then
            error('Error closing connection on init failure:\n' ..
                'init failure: ' .. setup .. ',\n' ..
                'close failure: ' .. err .. '\n')
        else
            error(setup)
        end
    end

    if self.DEBUG > 0 then
        print('torchcraft:connect() finished, establishing command control.')
    end
    return setup
end

function torchcraft:set_variables()
    -- initializing the "game state" booleans
    self.state:reset()
    self.window_size = {200, 150}
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
    -- TODO? redo with pl.Set?
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
    return self.client:receive()
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
    self.client:send(t)
end

function torchcraft:close()
    self.client:close()
end

-- return a new torchcraft context
function torchcraft.new()
   local newtc = {}
   for k,v in pairs(torchcraft) do
      newtc[k] = v
   end
   newtc.client = torchcraft.Client()
   newtc.state = newtc.client.state
   newtc.mode = tablex.deepcopy(torchcraft.mode)  -- reset mode for new context
   newtc:set_variables()
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
    elseif air and utype == self.unittypes.Protoss_Scout then
        return 2
    else
        return 1
    end
end

function torchcraft:get_weapon(source, dest)
    local wd = 0
    local dmgtype = -1
    local range = 0
    local air = false
    if self.staticdata.isFlyer[dest.type] then
        wd = source.awattack -- n attacks and dmg multiplier accounted for
        dmgtype = source.awdmgtype
        range = source.awrange
        air = true
    else
        wd = source.gwattack -- n attacks and dmg multiplier accounted for
        dmgtype = source.gwdmgtype
        range = source.gwrange
    end
    return wd, dmgtype, range, air
end

function torchcraft:compute_dmg(source, dest)
    local wd, dmgtype, _, air = self:get_weapon(source, dest)
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
    local hpdmg = self:dmg_multiplier(dmgtype, dest.size)*wd
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
