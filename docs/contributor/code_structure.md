# Structure of the code

TorchCraft is the library that makes the glue between Torch and StarCraft, but
it is effectively made of several modules.

## BWEnv

`BWEnv/` includes all the code that effectively interfaces with StarCraft: Brood
War. It is largely based on [BWAPI](http://bwapi.github.io), and it provides
functionality to connect with TorchCraft (the lua library), send game data, and
receive commands.

Open `BWEnv/VisualStudio/BWEnv.sln` in VisualStudio 2013 to check the entire
project. The code can be compiled in four modes: `Release`, `Debug`,
`DLL-Release`, and `DLL-Debug`. The release modes are the ones released by us as
binaries, and they respectively output an executable and a DLL. The first one
can be launched and will completely control StarCraft, while the second will
have to be injected within StarCraft using for instance ChaosLauncher.


## Replayer

`replayer/` contains the code that serialises and deserialises the game data
both within the client and the server, both for communicating between StarCraft and Torch and for the experience replay (gamestore).


## TorchCraft

* `init.lua` contains the majority of the code that provides the lua interface.
 Most of it is accessible directly to the user after a `require 'torchcraft'`.
* `utils.lua` contains functions that might be useful to the user (but are most
 likely not required to use the library).
* `examples/` contains a few scripts that can run torchcraft immediately after installation:
  - `simple_[dll|exe].lua` simply run a standard reinforcement learning loop
  (use those to easily check the data that can be received).
  - `self_play_dll.lua` is a demo that shows how to run bots vs bots scenarios.
  - `visual_example.lua` allows to visualize simple visual encodings of the
  visible game state that can simplify visual-based training.
  - `baseline_heuristic.lua` contains a few simple heuristics for micro battles
  (and works similarly to `simple_dll` unless stated otherwise in the
  script).


## Non-code

 * `config/` contains configuration for the client side (the one that connects to
  StarCraft).
 * `maps/` contains a few maps that you can use to test our micromanagement examples.
