# TorchCraft

A bridge between [Torch](http://torch.ch/) and [StarCraft](http://us.blizzard.com/en-us/games/sc/).

A general overview of the system (and motivations for using TorchCraft) can be found in:

Synnaeve, G., Nardelli, N., Auvolat, A., Chintala, S., Lacroix, T., Lin, Z.,
Richoux, F. and Usunier, N., 2016. _TorchCraft: a Library for Machine Learning Research
on Real-Time Strategy Games_ - [arXiv:1611.00625](https://arxiv.org/abs/1611.00625).


## [Installation](docs/user/starcraft_in_windows.md)

Please follow the [installation instructions](docs/user/starcraft_in_windows.md). 

You will need to have a license of StarCraft: Brood War. 

TorchCraft is a BWAPI module that sends StarCraft data out over a ZMQ
connection. This lets you parse StarCraft data and interact with BWAPI from
anywhere. The TorchCraft client should be installed from C++, Python, or Lua.
We provide off the shelf solutions for Python and Lua:

Requirements:
  - zstd-devel 1.1.4
  - zeromq 4+

Remember to init submodules: `git submodule update --init --recursive`<br/>
Python setup: `pip install pybind11 && pip install .`<br/>
Lua setup (depends on [torch7](http://torch.ch/docs/getting-started.html)): `luarocks make *.rockspec`<br/>

We provide an example C++ CMake project in `examples/cpp/`

The hardest part of installing the server is actually setting up starcraft.
We detail three ways of doing this:
  - [Use the standard Windows + StarCraft route, perhaps with a VM](/docs/user/starcraft_in_windows.md)
  - [Use WINE](/docs/user/starcraft_in_wine.md)
  - [Use OpenBW](/docs/user/openbw.md)


## Running TorchCraft

See [`examples/`](examples/) for a list of scripts that you can try.
In all cases, the `$server_ip` is the ip address of the machine running
StarCraft.

### StarCraft Server

Do whichever was installed:

#### Windows / WINE

1. Open `bwapi.ini` and set `ai = PATH\TO\BWEnv.dll` (or copy
   ours from `config/bwapi.in`)
2. Start ChaosLauncher, enable `BWAPI DLL Injector [Release]`.
3. Press `Start`.


#### WINE

1. Open `bwapi.ini` and set `ai = PATH\TO\BWEnv.dll` (or copy
1. `wine bwheadless.exe -e $STARCRAFT/StarCraft.exe -l $STARCRAFT/bwapi-data/BWAPI.dll --headful` as detailed in the [WINE docs](/docs/user/starcraft_in_wine.md)


#### OpenBW

Something like:
`OPENBW_ENABLE_UI=0 BWAPI_CONFIG_AI__RACE=Terran BWAPI_CONFIG_AI__AI="BWEnv/build/BWEnv.so" BWAPI_CONFIG_AUTO_MENU__AUTO_MENU="SINGLE_PLAYER" BWAPI_CONFIG_AUTO_MENU__MAP=maps/micro/m5v5_c_far.scm BWAPI_CONFIG_AUTO_MENU__GAME_TYPE="USE MAP SETTINGS" TORCHCRAFT_PORT=11111 BWAPILauncher`

### TorchCraft Client

```bash
$ cd examples
$ th lua/simple_dll.lua -t $server_ip
$ python py/example.py -t $server_ip
```

## Other Documentation

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
