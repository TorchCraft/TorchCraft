#!/bin/bash

WINE_DIR=${WINEPREFIX:-~/.wine}
TC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [[ `uname` == 'Darwin' ]]; then
    SC_DIR="$(dirname $(find $WINE_DIR -name StarCraft.exe -print -quit))"
else
    SC_DIR="$(dirname $(readlink -f $(find $WINE_DIR -name StarCraft.exe -print -quit)))"
fi

SC_DIR=${STARCRAFT_DIR:-$SC_DIR}
if [ ! -e $SC_DIR ]; then
	echo "Cannot find StarCraft directory, try setting $STARCRAFT_DIR"
	exit 1
fi

cd $SC_DIR
cp $TC_DIR/BWEnv/bin/*.exe ./
cp $TC_DIR/BWEnv/bin/*.dll ./
cp $TC_DIR/BWEnv/bin/*.sh ./
cp $TC_DIR/config/*.ini $SC_DIR/bwapi-data/
cp $TC_DIR/maps/micro/ $SC_DIR/Maps/BroodWar/ -r
curl -LO https://github.com/TorchCraft/TorchCraft/releases/download/v1.0-0/torchcraft-v1.0-0.zip
unzip torchcraft-v1.0-0.zip
cp torchcraft-v1.0-0/* ./

if [[ `uname` == 'Darwin' ]]; then
    sed -i '' "s|^ai *=.*|ai=$SC_DIR/BWEnv.dll|" bwapi-data/bwapi.ini
else
    sed -i "s|^ai *=.*|ai=$SC_DIR/BWEnv.dll|" bwapi-data/bwapi.ini
fi

# Set up the right registry keys
cd $WINE_DIR
cp user.reg user.reg.BAK  # Make a backup
cat $TC_DIR/docker/common/regkeys >> user.reg
