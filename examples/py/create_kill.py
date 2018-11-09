# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
# 
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

import argparse
import random

import torchcraft as tc
import torchcraft.Constants as tcc

parser = argparse.ArgumentParser(
    description='Plays simple micro battles with an attack closest heuristic')
parser.add_argument(
    '-t', '--hostname', type=str, help='Hostname where SC is running')
parser.add_argument('-p', '--port', default=11111, help="Port to use")
parser.add_argument(
    '-b',
    '--micro_battles',
    action='store_true',
    help="Set to true if needing micro_mode")
parser.add_argument('-d', '--debug', default=0, type=int, help="Debug level")

args = parser.parse_args()

DEBUG = args.debug


def create_units(player, quantity, x=100, y=100, u_type=0):
    commands = []
    for i in range(quantity):
        command = [
            tcc.command_openbw,
            tcc.openbwcommandtypes.SpawnUnit,
            player,
            u_type,
            x,
            y,
        ]
        commands.append(command)
    return commands


def kill_units(units, quantity):
    commands = []
    random.shuffle(units)
    for i in range(quantity):
        u = units[i]
        command = [
            tcc.command_openbw,
            tcc.openbwcommandtypes.KillUnit,
            u.id,
        ]
        commands.append(command)
    return commands


skip_frames = 1
nrestarts = 0
total_battles = 0
max_add_quantity = 20
tries = 5

cl = tc.Client()
cl.connect(args.hostname, args.port)
state = cl.init(micro_battles=args.micro_battles)
returned = cl.send([
    [tcc.set_combine_frames, skip_frames],
    [tcc.set_speed, 0],
    [tcc.set_gui, 1],
    [tcc.set_cmd_optim, 1],
    [tcc.map_hack],
])
state = cl.recv()

for i in range(tries):
    print("# try: {}".format(i))

    while state.game_ended or state.waiting_for_restart:
        returned = cl.send([])
        state = cl.recv()

    unit_add_quantity = random.randint(1, max_add_quantity)

    initial_lens = [0, 0]
    for i in range(2):
        initial_lens[i] = len(state.units[i])

    print("Current size: {}".format(initial_lens))
    command = create_units(0, unit_add_quantity)
    command += create_units(1, unit_add_quantity)

    print("Creating {} units...".format(unit_add_quantity))
    cl.send(command)
    state = cl.recv()

    assert (len(state.units[0]) == initial_lens[0] +
            unit_add_quantity), 'Player: {} =/= {}'.format(
                len(state.units[0]), initial_lens[0])
    assert (len(state.units[1]) == initial_lens[1] +
            unit_add_quantity), 'Enemy: {} =/= {}'.format(
                len(state.units[1]), initial_lens[1])

    print("Current size: {}".format([len(state.units[0]),
                                     len(state.units[1])]))

    command = kill_units(state.units[0], unit_add_quantity)
    command += kill_units(state.units[1], unit_add_quantity)

    print("Killing {} units...".format(unit_add_quantity))
    cl.send(command)
    state = cl.recv()

    assert (len(
        state.units[0]) == initial_lens[0]), 'Player: {} =/= {}'.format(
            len(state.units[0]), initial_lens[0])
    assert (len(state.units[1]) == initial_lens[1]), 'Enemy: {} =/= {}'.format(
        len(state.units[1]), initial_lens[1])
    print("Current size: {}".format(initial_lens))

cl.close()
print("All good.")
