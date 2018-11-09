# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
# 
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

import argparse
import torchcraft as tc
import torchcraft.Constants as tcc

parser = argparse.ArgumentParser(
    description='Plays simple micro battles with an attack closest heuristic')
parser.add_argument('-t', '--hostname', type=str,
                    help='Hostname where SC is running')
parser.add_argument('-p', '--port', default=11111,
                    help="Port to use")
parser.add_argument('-d', '--debug', default=0, type=int, help="Debug level")

args = parser.parse_args()

DEBUG = args.debug
def dprint(msg, level):
    if DEBUG > level:
        print(msg)

def get_closest(x, y, units):
    dist = float('inf')
    u = None
    for unit in units:
        d = (unit.x - x)**2 + (unit.y - y)**2
        if d < dist:
            dist = d
            u = unit
    return u

maps = [
    'Maps/BroodWar/micro/dragoons_zealots.scm',
    'Maps/BroodWar/micro/m5v5_c_far.scm'
]

skip_frames = 7
nrestarts = 0
total_battles = 0

while total_battles < 40:
    print("")
    print("CTRL-C to stop")
    print("")

    battles_won = 0
    battles_game = 0
    frames_in_battle = 1
    nloop = 1
    nrestarts += 1

    cl = tc.Client()
    cl.connect(args.hostname, args.port)
    state = cl.init(micro_battles=True)
    for pid, player in state.player_info.items():
        print("player {} named {} is {}".format(player.id, player.name,
            tc.Constants.races._dict[player.race]))

    # Initial setup
    cl.send([
        [tcc.set_speed, 0],
        [tcc.set_gui, 1],
        [tcc.set_cmd_optim, 1],
    ])
    while not state.game_ended:
        nloop += 1
        state = cl.recv()
        actions = []
        if state.game_ended:
            break
        elif state.battle_just_ended:
            dprint("BATTLE ENDED", 0)
            if state.battle_won: battles_won += 1
            battles_game += 1
            total_battles += 1
            frames_in_battle = 0
            if battles_game >= 10:
                actions = [
                    [tcc.set_map, maps[nrestarts % len(maps)], 0],
                    [tcc.quit],
                ]
                print(maps[nrestarts % len(maps)])
        elif state.waiting_for_restart:
            dprint("WAITING FOR RESTART", 0)
        else:
            myunits = state.units[0]
            enemyunits = state.units[1]
            if state.battle_frame_count % skip_frames == 0:
                for unit in myunits:
                    target = get_closest(unit.x, unit.y, enemyunits)
                    if target is not None:
                        actions.append([
                            tcc.command_unit_protected,
                            unit.id,
                            tcc.unitcommandtypes.Attack_Unit,
                            target.id,
                        ])
        if frames_in_battle > 2 * 60 * 24:
            actions = [[tc.quit]]
            nrestarts += 1
        dprint("Sending actions: " + str(actions), 1)
        cl.send(actions)
    cl.close()
