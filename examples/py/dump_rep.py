# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
# 
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

import torchcraft as tc
import torchcraft.Constants as tcc
import argparse
import os.path as path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import sys

parser = argparse.ArgumentParser(
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
parser.add_argument('-t', '--hostname', default="localhost", type=str,
                    help="Server to connect to")
parser.add_argument('-p', '--port', default=11111, type=str,
                    help="Port for torchcraft")
parser.add_argument('-o', '--out', default="/tmp", 
                    help="Where to save the replay")
args = parser.parse_args()

skip_frames = 3

cl = tc.Client()
cl.connect(args.hostname, args.port)
state = cl.init()
cl.send([
    [tcc.set_speed, 0],
    [tcc.set_gui, 0],
    [tcc.set_combine_frames, skip_frames, skip_frames],
    [tcc.set_max_frame_time_ms, 0],
    [tcc.set_blocking, 0],
    [tcc.set_frameskip, 1000],
    [tcc.set_log, 0],
    [tcc.set_cmd_optim, 1],
])
state = cl.recv()

rep = tc.replayer.Replayer()
rep.setMapFromState(state)
fc = 0
while not state.game_ended:
    rep.push(state.frame)
    fc += 1
    if fc > 5000:
        break
    state = cl.recv()

print("Game ended....")
savePath = path.join(args.out, state.map_name + ".tcr")

print("Saving to " + savePath)
rep.setKeyFrame(-1)
rep.save(savePath, True)
print('Done dumping replay')
cl.send([[tcc.quit]])

savedRep = tc.replayer.load(savePath)
savedMap = savedRep.getMap()
def checkMap(ret, correct, desc, outname):
    if (ret != np.asarray(correct).reshape(ret.shape)).sum() > 0:
        print("{} map doesn't match, replayer is bugged!".format(desc))
    plt.imshow(ret, cmap='hot', interpolation='nearest')
    plt.savefig(outname)
    plt.clf()
checkMap(savedMap['walkability'], state.walkable_data, "Walkability", "/tmp/walkmap.png")
checkMap(savedMap['ground_height'], state.ground_height_data, "Ground Height", "/tmp/heightmap.png")
checkMap(savedMap['buildability'], state.buildable_data, "Buildability", "/tmp/buildmap.png")

if len(savedMap['start_locations']) != len(state.start_locations):
  print("Not the same number of start locations, replayer is bugged")

slset = set(savedMap['start_locations'])
trueslset = set((p.x, p.y) for p in state.start_locations)
if slset != trueslset:
    print("Start locations are bugged, replay has: ")
    print(slset)
    print("While it should have: ")
    print(trueslset)

for i in range(len(rep)):
  f1 = rep.getFrame(i)
  f2 = savedRep.getFrame(i)
  good = f1.deepEq(f2)
  if not good:
    print("Saving failed! Frame {} doesn't match replay".format(i))
    sys.exit(1)

    

def write_creep_map(framenum, outname):
    f = savedRep.getFrame(framenum)
    plt.imshow(f.creep_map(), cmap='hot', interpolation='nearest')
    plt.savefig(outname)
    plt.clf()
write_creep_map(1, "/tmp/creep_map0.png")
write_creep_map(999, "/tmp/creep_map1.png")
write_creep_map(1999, "/tmp/creep_map2.png")
write_creep_map(2999, "/tmp/creep_map3.png")
write_creep_map(3999, "/tmp/creep_map4.png")
write_creep_map(4999, "/tmp/creep_map5.png")
