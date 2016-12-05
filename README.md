# TorchCraft

A bridge between [Torch](http://torch.ch/) and [StarCraft](http://us.blizzard.com/en-us/games/sc/).

A general overview of the system (and motivations for using TorchCraft) can be found in:

Synnaeve, G., Nardelli, N., Auvolat, A., Chintala, S., Lacroix, T., Lin, Z.,
Richoux, F. and Usunier, N., 2016. _TorchCraft: a Library for Machine Learning Research
on Real-Time Strategy Games_ - [arXiv:1611.00625](https://arxiv.org/abs/1611.00625).


## [Installation](docs/user/installation.md)

Please follow the [installation instructions](docs/user/installation.md). 

You will need to have a license of StarCraft: Brood War. 

The most tested and stable way to set it up is to have StarCraft running on Windows 
and Torch running in Linux or Mac OS X. If you already have StarCraft, you need to
install [BWAPI](docs/user/installation.md#bwapi),
[copy a few files](docs/user/installation.md#torchcraft-common-prerequisites), 
and either launch the [AIModule (DLL)](docs/user/installation.md#torchcraft-aimodule-dll-for-users) 
OR launch the [AIClient (EXE)](docs/user/installation.md#torchcraft-aiclient-exe-for-users). 

You can also run StarCraft on Linux through Wine, but this may be a bit more 
complicated, start with 
[BWAPI on Linux](https://github.com/TorchCraft/TorchCraft/blob/master/docs/user/bwapi_on_linux.md).


## Running TorchCraft

See [`examples/`](examples/) for a list of scripts that you can try.
The simplest examples come in two flavors: **DLL/AIModule** based and **EXE/AIClient**
based. In all cases, the `$server_ip` is the ip address of the machine running
StarCraft.

### DLL/AIModule TorchCraft

#### Windows

1. Open `bwapi.ini` and set `ai = PATH\TO\BWEnv.dll` (or copy
   ours from `config/bwapi.in`)
2. Start ChaosLauncher, enable `BWAPI DLL Injector [Release]`.
3. Press `Start`.

### Linux

1. Run

```bash
$ cd examples
$ th simple_dll.lua -t $server_ip
```

### EXE/AIClient TorchCraft

#### Windows

1. Start `BWEnv.exe`.

#### Linux

2. Run

```bash
$ cd examples
$ th simple_exe.lua -t $server_ip
```

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
