
# TorchCraft on OpenBW

With OpenBW, TorchCraft *and* StarCraft can run natively on Linux (or any other system), without the need for Virtual Machines or Wine.

### Dependencies
libzmq, openbw/bwapi (https://github.com/openbw/bwapi)

These instructions assume BWAPI is installed. If you installed to a non-default directory, then the path can be specified by passing `-DBWAPI_DIR=/new/path/` to cmake.

### Compile TorchCraft

```
$ apt-get install libczmq-dev
$ cd BWEnv; mkdir -p build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=relwithdebinfo && make -j
```
This will build a shared object BWEnv.so, which can be loaded by BWAPI. It will also produce a BWEnvClient executable, but **only the shared object will work with OpenBW**.

### Running

For detailed instructions on using BWAPILauncher, see https://github.com/openbw/bwapi.

*NOTE* You must have 

TL;DR is run BWAPILauncher from the StarCraft folder, settings are the same as usual (bwapi-data/bwapi.ini and bwapi-data/torchcraft.ini. These can be copied from TorchCraft/config/).

BWAPI settings can be specified through environment variables, eg
`BWAPI_CONFIG_AI__AI=/the/path/to/build/BWEnv.so BWAPILauncher`

If running from the `torchcraft` root directory, this would start one of the micro maps we've provdied:
`OPENBW_ENABLE_UI=0 BWAPI_CONFIG_AI__RACE=Terran BWAPI_CONFIG_AI__AI="BWEnv/build/BWEnv.so" BWAPI_CONFIG_AUTO_MENU__AUTO_MENU="SINGLE_PLAYER" BWAPI_CONFIG_AUTO_MENU__MAP=maps/micro/m5v5_c_far.scm BWAPI_CONFIG_AUTO_MENU__GAME_TYPE="USE MAP SETTINGS" TORCHCRAFT_PORT=11111 BWAPILauncher`
