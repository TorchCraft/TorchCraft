# TorchCraft

TorchCraft is an interface to StarCraft: Brood War based on [BWAPI](github.com/bwapi/bwapi) that allows you to build bots and control the game without needing to inject your own controller into the game. This lets you parse StarCraft data and interact with BWAPI from anywhere with a few lines of C++, Python, or Lua.

A general overview of the system (and motivations for using TorchCraft) can be found in: Synnaeve, G., Nardelli, N., Auvolat, A., Chintala, S., Lacroix, T., Lin, Z., Richoux, F. and Usunier, N., 2016. _TorchCraft: a Library for Machine Learning Research on Real-Time Strategy Games_ - [arXiv:1611.00625](https://arxiv.org/abs/1611.00625). If you use TorchCraft in your research, please cite the paper.


## Installation

Requirements:
  - StarCraft: Brood War 1.16
  - A license of StarCraft: Brood War
  - zstd-devel 1.1.4
  - zeromq 4+


First, clone the repository and its submodules:

```bash
$ git clone https://github.com/torchcraft/TorchCraft --recursive

# o if you have already cloned the repo...
$ git submodule update --init --recursive
```


TorchCraft is split into two parts: **BWEnv**, the process that runs along with StarCraft, and the set of client interfaces implemented in C++, Python, and Lua.

- To install BWEnv on an instance of StarCraft running on Windows, please follow these [installation instructions](docs/user/starcraft_in_windows.md). 
- To install BWEnv and StarCraft on WINE, please check these [instructions](docs/user/starcraft_in_wine.md).
- To install BWEnv using OpenBW, please check follow these [instructions](docs/user/openbw.md).


Each client has a different installation procedure, but all have the same library requirements (see above).


### Python

This should work with a standard python>=3.5 installation or Anaconda, both on Linux and MacOS:

```bash
# install requirements
$ pip install pybind11
$ pip install .
```

### Lua

The lua setup depends on [torch7](http://torch.ch/docs/getting-started.html).

```bash
$ luarocks make *.rockspec
```
Lua setup (depends on ): `luarocks make *.rockspec`<br/>


### C++

We provide an example C++ CMake project in `examples/cpp/`, with the additional dependency of `gflags`.

```bash
$ cd examples/cpp
$ mkdir build; cd build
$ cmake ..
$ make
```


## Running TorchCraft

See [`examples/`](examples/) for a list of scripts that you can try. In all cases, the `$server_ip` is the ip address of the machine running
StarCraft.


### StarCraft Server

Do whichever was installed:

#### Windows / WINE

1. Open `bwapi.ini` and set `ai = PATH\TO\BWEnv.dll` (or copy ours from `config/bwapi.ini`)
2. Start ChaosLauncher, enable `BWAPI DLL Injector [Release]`.
3. Press `Start`.


#### WINE

1. Open `bwapi.ini` and set `ai = PATH\TO\BWEnv.dll` (or copy
2. `wine bwheadless.exe -e $STARCRAFT/StarCraft.exe -l $STARCRAFT/bwapi-data/BWAPI.dll --headful` as detailed in the [WINE docs](/docs/user/starcraft_in_wine.md)


#### OpenBW

After installing openbw and building BWEnv, run from the torchcraft repo:

```bash
OPENBW_ENABLE_UI=0 \
  BWAPI_CONFIG_AI__RACE=Terran \
  BWAPI_CONFIG_AI__AI="BWEnv/build/BWEnv.so" \
  BWAPI_CONFIG_AUTO_MENU__AUTO_MENU="SINGLE_PLAYER" \
  BWAPI_CONFIG_AUTO_MENU__MAP=maps/micro/m5v5_c_far.scm \
  BWAPI_CONFIG_AUTO_MENU__GAME_TYPE="USE MAP SETTINGS" \
  TORCHCRAFT_PORT=11111 \
  BWAPILauncher
```

Remember that you'll need the StarCraft: Brood War MPQs from your local installation in the same directory. You can also setup these values in `bwapi.ini` and `torchcraft.ini`.


#### Running micromanagement example

In all micromanagement examples, the client has to be initialised with `micro_battles=True`. This allows the client to group states into "battles" using our "micro" maps. All such maps have triggers that recreate the initial state automatically after every battle is finished.

```bash
$ cd examples

# python
$ python py/example.py -t $server_ip

# lua
$ th lua/simple_dll.lua -t $server_ip
```

Note that we have provide an empty map in which you can build your own scenarios using openbw (see `maps/empty.scm` and `examples/py/create_kill.py`), without relying on triggers and `micro_battles=True`.


## Other Documentation

The code is for the most self-documenting, these documents go a bit more in detail on certain parts of TorchCraft:

- [Client basics](/docs/user/torchcraft.md)
- [Lua Replayer](/docs/user/replayer.md)


## Citation

Please cite the arXiv paper if you use TorchCraft in your work:

```
@article{synnaeve2016torchcraft,
  title={TorchCraft: a Library for Machine Learning Research on Real-Time Strategy Games},
  author={Synnaeve, Gabriel and Nardelli, Nantas and Auvolat, Alex and Chintala, Soumith and Lacroix, Timoth{\'e}e and Lin, Zeming and Richoux, Florian and Usunier, Nicolas},
  journal={arXiv preprint arXiv:1611.00625},
  year={2016}
}
```

## Contributing

See [Contributing](CONTRIBUTING.md)

Also check out the [code structure](/docs/contributor/code_structure.md)
and [PR process](/docs/contributor/pr_process.md)
