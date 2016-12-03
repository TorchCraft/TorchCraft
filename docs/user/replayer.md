# TorchCraft Replayer Documentation #

The TorchCraft replayer is a C++ library with Lua bindings for manipulating
StarCraft game state storage.

Here we describe the Lua replayer API; however, the replayer can
also be used directly from C++.

The file [init.cpp](/replayer/init.cpp) registers the c++ functions to be exposed to Lua.

## `torchcraft.Frame`

The `torchcraft.Frame` is the basic element of game storage in torchcraft:
it stores a single frame of a game. It contains the full state of the game:
players, units, actions, bullets. It may also contain reward information to
be used for a reinforcement learning algorithm.


### `replayer.frameFromTable(table)`

Returns a `torchcraft.Frame` from a Lua table that has the following
structure:

```
Insert description here
```

### `replayer.frameFromString(string)`

Returns a `torchcraft.Frame` from unserializing a string. This is used
primarily by TorchCraft for receiving data from `starcraft.exe`.

### `frame:toTable()`

Returns a Lua table containing all the frame information
(see `replayer.frameFromTable`).

### `frame:getUnits(player_id)`

Returns a Lua table containing all the units of the given player.
The table is indexed by unit ID (`uid`).

### `frame:getNumPlayers()`

Return the number of players participating in the game.

### `frame:getNumUnits()`

Returns the total number of units in the frame.

### `frame:getNumUnits(player_id)`

Return the number of units in the frame belonging to given player.

### `frame:combine(next_frame)`

Combine the `next_frame` with the current frame. This is useful when frame-
skipping to avoid missing valuable information during the frames you skip.

Effectively, commands and units are accumulated (so we retrieve all the orders
received during the both frames). Please beware that if a unit of current frame
is dead in `next_frame`, it will still be present in the combined frame (with its
last known stats). If you want to remove the dead units, you have to filter
them by hand afterward (using `torchcraft.state.deaths`)

> Reward, bullets, actions and terminal state are NOT accumulated (we keep the
  value of the next frame).

## `torchcraft.Replayer`

The `torchcraft.Replayer` class is meant to contain sequences of frames,
storing a whole game or only part of it. It also optionally stores map
data for the game.

### `replayer.newReplayer()`

Creates empty replayer containing no frames.

### `replayer.loadReplayer(filename)`

Loads replayer from file, restoring whole game state.

### `replayer:save(filename)`

Saves whole game state from replayer to file.

### `replayer:getNumFrames()`

Returns number of frames in the game.

### `replayer:getFrame(n)`

Returns frame `n`, as a `torchcraft.Frame`

### `replayer:getMap()`

Returns the map data for the game (a 2D Torch ByteTensor).
The map contains height information as follows:
  - 0: Low ground
  - 1: Low ground doodad
  - 2: High ground
  - 3: High ground doodad
  - 4: Very high ground
  - 5: Very high ground doodad

A "doodad" is simply a special object in this position. It is purely graphical,
and no interaction is possible with it. Sometimes, the tile is still walkable.

If the tile is not walkable, then the corresponding cell contains 255 (aka -1
  with an underflow)


### `replayer:setMap(map_data)`

Set the map data for the game (a 2D Torch ByteTensor).

### `replayer:push(frame)`

Append a `torchcraft.Frame` to the replayer.


## `torchcraft.GameStore`

The `torchcraft.GameStore` class is meant to store many replays, categorized
as either "won" or "lost". This class is used in reinforcement learning
algorithms to do experience replay.

### `replayer.newGameStore(won, lost)`

Creates a new empty game store, preallocating space for a given number of
won and lost games.

### `replayer.loadGameStore(filename)`

Load gamestore from file.

### `gamestore:save(filename)`

Save gamestore to file.

### `gamestore:add(game, won)`

Add game to gamestore, either in the won set if won is true or in the lost
set otherwise. The game must be a `torchcraft.Replayer`.

### `gamestore:sample(prop_won)`

Sample a game from the gamestore, with probability `prop_won` of selecting
a won game. Returns a `torchcraft.Replayer`.

### `gamestore:getSizeLost()`

Return the number of lost games in the gamestore.

### `gamestore:getLastBattlesLost(n)`

Return the `n` last lost battles in the gamestore.

### `gamestore:getLast()`

Return the last battle in the gamestore, taking it from the lost or won set
accordingly.
