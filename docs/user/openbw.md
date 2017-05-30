
# TorchCraft on OpenBW

With OpenBW, TorchCraft *and* StarCraft can run natively on Linux (or any other system), without the need for Virtual Machines or Wine.

Note: This is an early version, not all unit commands are supported, some maps or map triggers might not work, there might be other bugs. The complete map information BWAPI flag is always enabled (you can always see all units, even if they are in fog of war).

### Dependencies
czmq, sdl2 (optional)

These can be installed on ubuntu with apt install libczmq-dev libsdl2-dev

### Compile

```
$ git clone https://github.com/TorchCraft/TorchCraft.git
$ git clone https://github.com/openbw/openbw.git
$ cd TorchCraft/BWEnv
$ mkdir build
$ cd build
$ OPENBW_DIR=../../../openbw cmake .. -DOPENBW_ENABLE_UI=ON
$ make
```
This should build a bwenv executable. If you omit the -DOPENBW_ENABLE_UI=ON part, then openbw will be built with no graphics support, and SDL2 will not be required.
Otherwise, to get any on screen visualization TorchCraft needs to send the set_gui command, eg `tc.command(tc.set_gui, 1)`. The examples do this. If it is omitted, or if `tc.command(tc.set_gui, 0)` is run then nothing will appear on screen.

### Running

OpenBW needs Stardat.mpq, Broodat.mpq and Patch_rt.mpq from StarCraft to run. These must be in the current directory when bwenv is run.

The easiest is to run bwenv from the StarCraft folder (since the mpq files will be there). Both StarCraft 1.16.1 and 1.18 can be used.

./bwapi-data/bwapi.ini and ./bwapi-data/torchcraft.ini must also exist, and can be copied from TorchCraft/config, by eg. `cp -r ~/TorchCraft/config ~/StarCraft/bwapi-data`

`~/StarCraft# ~/TorchCraft/BWEnv/build/bwenv` will run OpenBW and TorchCraft should be able to connect to it (you might need to use 127.0.0.1 instead of localhost as hostname).

bwapi.ini, torchcraft.ini configuration and environment variables is the same as usual.
