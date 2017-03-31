--[[
-- This is a replay dumper test / example. Simply set your starcraft to open a
-- replay. Then, run
--  th dump_replay -t $SC_IP
]]
require 'torch'
torch.setdefaulttensortype('torch.FloatTensor')
require 'sys'
require 'sys'
local lapp = require 'pl.lapp'
local params = lapp [[
Tests replay dumping / reloading
-t,--hostname       (default "")    Give hostname / ip pointing to VM
-p,--port           (default 11111) Port for torchcraft. Do 0 to grab vm from cloud
-o,--out            (default "/tmp") Where to save the replay
]]

local skip_frames = 3
local port = params.port
local hostname = params.hostname or ""
print("hostname:", hostname)
print("port:", port)

local torch = require 'torch'
local threads = require 'threads'

local p = require 'pl.path'
local replayer = require 'torchcraft.replayer'
local tc = require 'torchcraft'
tc:init(params.hostname, params.port)


print("Doing replay...")
local map = tc:connect(params.hostname, params.port)
assert(tc.state.replay, "This game isn't a replay")
tc:send({table.concat({
  tc.command(tc.set_speed, 0), tc.command(tc.set_gui, 0),
  tc.command(tc.set_combine_frames, skip_frames),
  tc.command(tc.set_frameskip, 1000), tc.command(tc.set_log, 0),
}, ':')})
tc:receive()
map = tc.state

local game = replayer.newReplayer()
game:setMap(map)
print('Dumping '..map.map_name)

local is_ok, err = false, nil
while not tc.state.game_ended do
  is_ok, err = pcall(function () return tc:receive() end)
  if not is_ok then break end
  game:push(tc.state.frame)
end

print("Game ended....")
local savePath = params.out.."/"..map.map_name..".tcr"
if not is_ok then
  print('Encountered an error: ', err)
else
  print("Saving to " .. savePath)
  game:setKeyFrame(-1)
  game:save(savePath, true)
  print('Done dumping replay')
  tc:send({table.concat({tc.command(tc.quit)}, ":")})
end

tc:close()


local savedRep = replayer.loadReplayer(savePath)
walkmap, heightmap, buildmap, startloc = savedRep:getMap()

function checkMap(ret, correct, desc, outname)
  if ret:ne(correct):sum() ~= 0 then
    print(desc .. " map doesn't match!, replayer is bugged!")
  end
  local mf = io.open(outname, 'w')
  local max = ret:max()
  mf:write("P2 " .. walkmap:size(2) .. " " .. walkmap:size(1) .. " " .. max .. "\n")
  for y = 1, ret:size(1) do
    for x = 1, ret:size(2) do
      mf:write(ret[y][x] .. " ")
    end
    mf:write('\n')
  end
end
checkMap(walkmap, map.walkable_data, "Walkability", "/tmp/walkmap.pgm")
checkMap(heightmap, map.ground_height_data, "Ground Height", "/tmp/heightmap.pgm")
checkMap(buildmap, map.buildable_data, "Buildability", "/tmp/buildmap.pgm")

if #startloc ~= #map.start_locations then
  print("Not the same number of start locations, replayer is bugged")
end
saw = {}
for i, _ in ipairs(startloc) do saw[i] = false end
for _, p in ipairs(startloc) do
  found = false
  for i, p2 in ipairs(map.start_locations) do
    if p.x == p2.x and p.y == p2.y and saw[i] == false then
      saw[i] = true
      found = true
    end
  end
  if not found then
    print("Start location ("..p.x..","..p.y..") is incorrect, replayer is bugged!")
    return
  end
end
local first = game:getFrame(1)
for i=1, game:getNumFrames() do
  local f1 = game:getFrame(i)
  local f2 = savedRep:getFrame(i)
  local good = f1:deepEq(f2)
  if not good then
    print("Saving failed! Frame " .. i .. " doesn't match replay")
    return
  end
  if i > 240 then
    local eq = f1:deepEq(first, false) -- false suppresses debug info
    if eq then
      print("Why is frame " .. i .. " the exact same as frame 1?")
      return
    end
  end
end
print("Saving succeeded!")
