import argparse
import pathlib
import numpy as np
import matplotlib.pyplot as plt

import torchcraft as tc
import torchcraft.Constants as tcc


parser = argparse.ArgumentParser(
    description='Visualise torchcraft image states')
parser.add_argument(
    '-t',
    '--hostname',
    type=str,
    help='Hostname where SC is running',
    default='localhost')
parser.add_argument('-p', '--port', default=11111, help="Port to use")

args = parser.parse_args()

skip_frames = 15

dir_path = "./visual_example"
try:
    p = pathlib.Path(dir_path)
    p.mkdir(exist_ok=True)
    print("Create directory: {}".format(dir_path))
except FileExistsError:
    print("Found directory: {}".format(dir_path))

print("")
print("CTRL-C to stop")
print("")

cl = tc.Client()
cl.connect(args.hostname, args.port)
state = cl.init(micro_battles=True)
for pid, player in state.player_info.items():
    print("player {} named {} is {}".format(
        player.id, player.name,
        tc.Constants.races._dict[player.race]))

# Initial setup
cl.send([
    [
        tcc.command_openbw,
        tcc.openbwcommandtypes.SetScreenValues,
        500,  # x
        900,  # y
        400,  # width
        500,  # height
    ],
    [tcc.set_combine_frames, skip_frames],
    [tcc.set_speed, 0],
    [tcc.set_gui, 1],
    [tcc.set_cmd_optim, 1],
    [tcc.request_image, 1],
])

nloop = 0

while not state.game_ended:
    nloop += 1
    state = cl.recv()
    actions = []
    if state.game_ended:
        break
    elif state.battle_just_ended:
        print("BATTLE ENDED")
        actions = [
            [tcc.quit],
        ]
    elif state.waiting_for_restart:
        print("WAITING FOR RESTART")
    else:
        the_image = np.array(state.image, dtype=np.uint8)
        # reshape is C, H, W
        the_image = the_image.reshape(3, state.image_size[0],
                                      state.image_size[1])
        # matplotlib likes HWC
        the_image = the_image.transpose(1, 2, 0)
        plt.imshow(the_image)
        fig = plt.gcf()

        savepath = '{}/visual_output_{}.png'.format(dir_path, nloop)
        plt.savefig(savepath)
        print('Saved image at {}'.format(savepath))
    actions += [[tcc.request_image, 1]]
    cl.send(actions)
cl.close()
