# BWAPI on Linux

## Instructions
### 1. Installing Wine
- On Ubuntu (confirmed on 14.04 and 16.04)
  -  `sudo apt-get install wine` 
- On Mac (confirmed on 10.12.1)
  -  `brew install wine winetricks`

Warning: Chaoslauncher was only tested to work on wine1.6. For most non-ubuntu distributions you must compile from scratch. However, you can still use the server through bwheadless.


## Installing Starcraft

- Download the game
  at [https://eu.battle.net/](https://battle.net/account/management/)
- Wait for StarCraft to download Wine
  `~/Downloads/Downloader_StarCraft_Combo_enGB.exe`
- Run `setup.exe` and install the game in some folder (`C:/StarCraft` is
  suggested/) via Wine
  - `$STARCRAFT` will be from now on the installation directory
    `export STARCRAFT=~/.wine/drive_c/StarCraft`
- Download 1.16.1 patch at
  [https://eu.battle.net/support/en/article/classic-game-patches](https://eu.battle.net/support/en/article/classic-game-patches)
  - Make sure you download the **Brood War** patch.
- `wine BW-1161.exe` 
- Enjoy a game :tada:


## Installing Dependencies

    winetricks -q vcrun2013
    
    # Download latest release of BWAPI at https://github.com/bwapi/bwapi/releases
    wget https://github.com/bwapi/bwapi/releases/download/v4.1.2/BWAPI_412_Setup.exe
    wine BWAPI_412_Setup.exe
    # INSTALL THIS TO C:\StarCraft\BWAPI
    
    # Install TorchCraft
    cd $STARCRAFT
    git clone https://github.com/TorchCraft/TorchCraft.git
    bash TorchCraft/quick_setup.sh


## Running AIModule

    cd $STARCRAFT
    wine bwheadless.exe -e $STARCRAFT/StarCraft.exe -l $STARCRAFT/bwapi-data/BWAPI.dll --headful


## Running AIClient

    cd $WINE_WORKSPACE
    wine BWEnv.exe


## Notes

- You can just copy a copy of Starcraft over from a Windows installation if you want.
  - BWAPI should be installed into StarCraft/BWAPI
  - Registry keys need playing with: default settings can be applied with 
    `regedit docker/common/regkeys`. If you messed around with the install path,
    then system.reg should have something like: 
    ```
    [Software\\Wow6432Node\\Blizzard Entertainment\\Starcraft] 1478814272
    "GamePath"="C:\\StarCraft\\Starcraft.exe"
    "InstallPath"="C:\\StarCraft\\"
    ```
- Other operating systems:
  - Sometimes, you need to emulate a virtual desktop: `$ wine /desktop=foo,
  800x600 bwheadless.exe -e $STARCRAFT/StarCraft.exe -l bwapi-data/BWAPI.dll 
  --headful`
- If the auto-menu doesn’t work for maps:
  - Run everything from the $STARCRAFT folder
  - Make sure you have vcrun2013 installed through winetricks
  - Make sure `winecfg` shows that `msvcp120` and `msvcr120` are being used, and other
    versions of msvcp are not.
