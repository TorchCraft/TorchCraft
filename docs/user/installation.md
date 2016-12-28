Terminology:
- Server = the part of the code that runs along with StarCraft (that is where we instantiate a ZMQ server).
- Client = the part of the code that runs in Torch, consumes data (state) from the server and sends commands (actions).

Installation of TorchCraft is based on your platform and whether you are using AIClient or AIModule.

# Windows Server

## System Requirements

- Windows 7 (XP and 8.1 are unsupported but should work, 10 did not in our last test)
- Visual Studio 2013 (for developpers / contributors only, the Community edition is sufficient)
- Starcraft (See [below](#starcraft-1161) for installation instructions)
- BWAPI (See [below](#bwapi) for installation instructions)

## Installation

### StarCraft (1.16.1)

- Register on http://battle.net/.
- Buy License at https://battle.net/shop/en-us/product/starcraft.
- Download the game at https://battle.net/account/management/.
- Run `setup.exe` and install the game in some folder.
  - `$STARCRAFT` will be from now on the installation directory.
- Download the 1.16.1 patch at https://battle.net/support/en/article/classic-game-patches.
  - Make sure you download the **Brood War** patch.
- Install the patch.

  > The patch should be discovered automatically, but if it's not, make sure to point it to the right folder.

- Enter the game and create a character.
  - Single Player → Expansion, AND
  - Multiplayer → Expansion → Local PC
- Enjoy a game :)


### BWAPI

- Download latest release at https://github.com/bwapi/bwapi/releases.
- Install this in `$STARCRAFT\BWAPI`.


### TorchCraft, common prerequisites:

- Copy `$STARCRAFT/TorchCraft/config/bwapi.ini` in `$STARCRAFT/bwapi-data/bwapi.ini`.
- Copy `$STARCRAFT/TorchCraft/config/torchcraft.ini` in `$STARCRAFT/bwapi-data/torchcraft.ini`.
- Copy `$STARCRAFT/TorchCraft/BWEnv/bin/*.dll` into `$STARCRAFT/`.
- Copy `$STARCRAFT/TorchCraft/maps/*` into `$STARCRAFT/Maps/BroodWar`.

Now you only need one of the following two methods, either AIModule (DLL) or AIClient (EXE).

### TorchCraft AIModule (DLL) for users:

- Extract `BWEnv.dll` from [this archive](https://github.com/TorchCraft/TorchCraft/releases/download/v1.0-0/torchcraft-v1.0-0.zip) and put it in `$STARCRAFT`
- Run `$STARCRAFT/BWAPI/ChaosLauncher/Chaoslauncher - MultiInstance.exe` **as administrator**.
- Check the “RELEASE” box from BWAPI.
- Click Start.
- Goto [Installing the Torch client (Linux)](#installing-the-torch-client-linux)


### TorchCraft AIClient (EXE) for users:

- Extract `BWEnv.exe` from [this archive](https://github.com/TorchCraft/TorchCraft/releases/download/v1.0-0/torchcraft-v1.0-0.zip) and put it in `$STARCRAFT`.
- Run `$STARCRAFT/BWEnv.exe`.
- Goto [Installing the Torch client (Linux)](#installing-the-torch-client-linux)


### TorchCraft for developers:

- Install Visual Studio 2013. The Community/Express version of Visual Studio is fine.
  - Follow the [same requirements as BWAPI](https://github.com/bwapi/bwapi/wiki/Compile-BWAPI).
- Clone TorchCraft into `$STARCRAFT/TorchCraft`.

  > We recommend Git Bash for windows: https://git-for-windows.github.io/).

- Open `$STARCRAFT/TorchCraft/BWEnv/VisualStudio/BWEnv.sln` and start hacking.
- Compile in `Release` mode for the AIClient (you will get a `BWEnv.exe` ) and in `DLL-Release` mode for the AIModule (you will get a `BWEnv.dll` ).


## Linux Server

Please see [BWAPI on Linux](/docs/user/bwapi_on_linux.md).

## Installing the Torch client (Linux)

### Torch

*(From http://torch.ch/docs/getting-started.html)*

In a terminal or CLI, type:

    $ curl -s https://raw.githubusercontent.com/torch/ezinstall/master/install-deps | bash
    $ git clone https://github.com/torch/distro.git ~/torch --recursive
    $ cd ~/torch; ./install.sh


### TorchCraft

    $ git clone git@github.com:torchcraft/torchcraft.git --recursive
    $ cd torchcraft
    $ luarocks make torchcraft-1.0-2.rockspec
    $ cd examples/
    $ th simple_{exe|dll}.lua -t $server_ip # depending on if you launched the exe or dll
