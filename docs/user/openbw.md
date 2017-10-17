
# TorchCraft on OpenBW

With OpenBW, TorchCraft *and* StarCraft can run natively on Linux (or any other system), without the need for Virtual Machines or Wine.

### Dependencies
libzmq, openbw/bwapi (https://github.com/openbw/bwapi)

These instructions assume BWAPI is installed. If you installed to a non-default directory, then the path can be specified by passing `-DBWAPI_DIR=/new/path/` to cmake.

### Compile

```
$ git clone https://github.com/TorchCraft/TorchCraft
$ cd TorchCraft/BWEnv
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=relwithdebinfo
$ make
```
This will build a shared object BWEnv.so, which can be loaded by BWAPI. It will also produce a BWEnvClient executable, but **only the shared object will work with OpenBW**.

### Running

For detailed instructions on using BWAPILauncher, see https://github.com/openbw/bwapi.

TL;DR is run BWAPILauncher from the StarCraft folder, settings are the same as usual (bwapi-data/bwapi.ini and bwapi-data/torchcraft.ini. These can be copied from TorchCraft/config/).

BWAPI settings can be specified through environment variables, eg
`BWAPI_CONFIG_AI__AI=/the/path/to/build/BWEnv.so BWAPILauncher`
