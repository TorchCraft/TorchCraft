--[[
   Copyright (c) 2015-present, Facebook, Inc.
   All rights reserved.
   This source code is licensed under the BSD-style license found in the
   LICENSE file in the root directory of this source tree. An additional grant
   of patent rights can be found in the PATENTS file in the same directory.
]]--

local tc = require 'torchcraft'
local utils = require 'torchcraft.utils'

local heuristics = {}

function heuristics.get_random(unitsTable)
    local n = 1
    local total = tablex.size(unitsTable)
    local rand = torch.rand(1):squeeze()
    local final = nil
    for uid, _ in pairs(unitsTable) do
        if n/total > rand then
            return uid
        end
        final = uid
        n = n+1
    end
    return final
end

function heuristics.get_random_action(pos, enemy_units)
    local moves = {}
    for m_x = -1, 1 do
        for m_y = -1, 1 do
            if m_x ~= 0 and m_y ~= 0 then
                table.insert(moves, {pos[1] + m_x * 16,  -- dont check
                                     pos[2] + m_y * 16}) -- boundaries
            end
        end
    end
    local rand = torch.random(1, tablex.size(enemy_units) + #moves)
    if rand > #moves then
        return get_random(enemy_units), nil
    end
    return nil, moves[rand]
end

function heuristics.get_weakest_and_closest(eut, mut)
    local mean_x = 0
    local mean_y = 0
    local n_mut = 0
    for uid, ut in pairs(mut) do
        mean_x = mean_x + ut.position[1]
        mean_y = mean_y + ut.position[2]
        n_mut = n_mut + 1
    end
    local mean_pos = {mean_x / n_mut, mean_y / n_mut}
    local min_total_HP = 1E30
    local min_dist = 1E30
    local weakest_uid = nil
    for uid, ut in pairs(eut) do
        local tmp_hp = ut['hp'] + ut['shield']
        if tmp_hp < min_total_HP then
            min_total_HP = tmp_hp
            weakest_uid = uid
        elseif tmp_hp == min_total_HP then
            local tmp_d = utils.distance(mean_pos, ut.position)
            if tmp_d < min_dist then
                min_dist = tmp_d
                weakest_uid = uid
            end
        end
    end
    return weakest_uid
end

function heuristics.dont_change_target(t, u, e)
    if u.target >= 0 and e[u.target] ~= nil
        and e[u.target].hp > 0 then
        return u.target
    else
        return t
    end
end

function heuristics.get_weakest_in_range_slack(eut, mut)
    local target = nil
    local pos = mut.position
    local gwr = mut.gwrange
    local awr = mut.awrange
    local bt = tc.xy_walktiles_per_buildtile
    for slack = 0, 8*bt, bt do
        if target ~= nil then
            break
        end
        local min_total_HP = 1E30
        local weakest_uid = nil
        for uid, ut in pairs(eut) do
            local tmp_d = utils.distance(pos, ut.position)
            if (tmp_d < (gwr + slack) and not isFlyer[ut.type])
                or (tmp_d < (awr + slack) and isFlyer[ut.type]) then
                local tmp_hp = ut['hp'] + ut['shield']
                if tmp_hp < min_total_HP then
                    min_total_HP = tmp_hp
                    weakest_uid = uid
                end
            end
        end
        target = weakest_uid
    end
    return target
end

function heuristics.get_weakest_in_range_slack_no_overkill(eunits,
        munits, changeok)
    local targets = {}
    local future_dmg = {}
    for uid, _ in pairs(eunits) do
        future_dmg[uid] = 0
    end
    for muid, mut in pairs(munits) do
        local target = nil
        if changeok == false then
            if mut.target > 0 and eunits[mut.target] ~= nil
                and eunits[mut.target].hp > 0 then
                target = mut.target
                local my_dmg = tc:compute_dmg(mut, eunits[target])
                future_dmg[target] = future_dmg[target] + my_dmg
            end
        end
        local pos = mut.position
        local gwr = mut.gwrange
        local awr = mut.awrange
        local bt = tc.xy_walktiles_per_buildtile
        for slack = 0, 8*bt, bt do
            if target ~= nil then
                break
            end
            local min_total_HP = 1E30
            local weakest_uid = nil
            for uid, ut in pairs(eunits) do
                local tmp_d = utils.distance(pos, ut.position)
                local tmp_hp = ut['hp'] + ut['shield']
                if ((tmp_d < (gwr + slack) and not isFlyer[ut.type])
                    or (tmp_d < (awr + slack) and isFlyer[ut.type]))
                    and future_dmg[uid] < tmp_hp then
                    if tmp_hp < min_total_HP then
                        min_total_HP = tmp_hp
                        weakest_uid = uid
                    end
                end
            end
            target = weakest_uid
        end
        if target ~= nil then
            local my_dmg = tc:compute_dmg(mut, eunits[target])
            future_dmg[target] = future_dmg[target] + my_dmg
        end
        targets[muid] = target
    end
    return targets
end

return heuristics
