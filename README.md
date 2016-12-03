# TorchCraft

A bridge between [Torch](http://torch.ch/) and [StarCraft](http://us.blizzard.com/en-us/games/sc/).

A general overview of the system (and motivations for using TorchCraft) can be found in:

Synnaeve, G., Nardelli, N., Auvolat, A., Chintala, S., Lacroix, T., Lin, Z., Richoux, F. and Usunier, N., 2016. _TorchCraft: a Library for Machine Learning Research on Real-Time Strategy Games_ - [arXiv:1611.00625](https://arxiv.org/abs/1611.00625).


## [Installation](docs/user/installation.md)


## Running TorchCraft

See [`examples/`](examples/) for a list of scripts that you can try.
The simplest examples come in two flavors: DLL based and server-client based.


### DLL TorchCraft

#### Windows

1. Open `bwapi.ini` and set `ai = PATH\TO\BWENV.dll` (or copy
   ours from `config/bwapi.in`)
2. Start ChaosLauncher, enable `BWAPI DLL Injector [Release]`.
3. Press `Start`.

### Linux

1. Run

```bash
$ cd examples
$ th simple_dll.lua
```

### Client TorchCraft

#### Windows

1. Start `BWEnv.exe`.

#### Linux

2. Run

```bash
$ cd examples
$ th simple_exe.lua
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
