# Installing TorchCraft

TorchCraft is split in two different parts:

* **Server**
  - process that runs along with StarCraft (that is where we instantiate a ZMQ
    server).
* **Client**
  - TorchCraft object in lua / c++ / python, which consumes data (state) from
    the server and sends commands (actions).

Installation of TorchCraft is based on your platform, and whether you are using
AIClient or AIModule.

**NOTA BENE: StarCraft 1.18 is currently not supported - see [bwapi/bwapi#710](https://github.com/bwapi/bwapi/issues/710).**

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
- Download the 1.16.1 patch at http://ftp.blizzard.com/pub/broodwar/patches/PC/BW-1161.exe
  - Make sure you download the **Brood War** patch 1.16.1.
- Install the patch.

  > The patch should be discovered automatically, but if it's not, make sure to point it to the right folder.

- **NOTE** It may be difficult to obtain 1.16.1 after the 1.18 release. 
  Check [here](https://us.battle.net/forums/en/starcraft/topic/20754525604) 
  for possible solutions. We will not support 1.18 until BWAPI does.
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

### TorchCraft AIModule (DLL) for users:

- Extract `BWEnv.dll` from the latest archive in the [release](https://github.com/TorchCraft/TorchCraft/releases/) 
  page  and put it in `$STARCRAFT`
- Run `$STARCRAFT/BWAPI/ChaosLauncher/Chaoslauncher - MultiInstance.exe` **as administrator**.
- Check the “RELEASE” box from BWAPI.
- Click Start.

### TorchCraft AIClient (EXE) for users (EXPERIMENTAL)

If you need support, please start an issue. The last time this was tested was 1.3.0.

- Extract `BWEnv.exe` from the latest archive in the [release](https://github.com/TorchCraft/TorchCraft/releases/) 
  page  and put it in `$STARCRAFT`.
- Run `$STARCRAFT/BWEnv.exe`.
- Goto [Installing the Torch client (Linux)](#installing-the-torch-client-linux)


### Developing the TorchCraft Server:

- Install Visual Studio 2013. The Community/Express version of Visual Studio is fine.
  - Follow the [same requirements as BWAPI](https://github.com/bwapi/bwapi/wiki/Compile-BWAPI).
- Clone TorchCraft into `$STARCRAFT/TorchCraft`.

  > We recommend Git Bash for windows: https://git-for-windows.github.io/).

- Set the `BWAPI_DIR` environment variable:
  - Right click on My Computer -> Properties -> 
    Advanced System Settings -> Environment Variables. Add `BWAPI_DIR` to be where
    you installed it, likely something like `C:\StarCraft\BWAPI`.
  - Restart the OS to apply the variable to the system.
- Open `$STARCRAFT/TorchCraft/BWEnv/VisualStudio/BWEnv.sln` and start hacking.
- Compile in `Release` mode for the AIClient (you will get a `BWEnv.exe` ) and in `DLL-Release` mode for the AIModule (you will get a `BWEnv.dll` ).


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
    $ luarocks make *.rockspec
    $ cd examples/
    $ th simple_{exe|dll}.lua -t $server_ip # depending on if you launched the exe or dll
