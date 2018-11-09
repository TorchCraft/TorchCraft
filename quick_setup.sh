#!/bin/bash
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
# 
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

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
curl -LO https://github.com/TorchCraft/TorchCraft/releases/download/v1.4-0/torchcraft-v1.4-0.zip
unzip torchcraft-v1.4-0.zip
cp torchcraft-v1.4-0/* ./
cp torchcraft-v1.4-0/bin/* ./

if [[ `uname` == 'Darwin' ]]; then
    sed -i '' "s|^ai *=.*|ai=$SC_DIR/BWEnv.dll|" bwapi-data/bwapi.ini
else
    sed -i "s|^ai *=.*|ai=$SC_DIR/BWEnv.dll|" bwapi-data/bwapi.ini
fi

# Set up the right registry keys
cd $WINE_DIR
cp user.reg user.reg.BAK  # Make a backup
cat $TC_DIR/docker/common/regkeys >> user.reg
