# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
# 
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

STARCRAFT_DIR=${WINEPREFIX:-~/.wine}/drive_c/StarCraft
if [ ! -e $STARCRAFT_DIR ]; then
  echo "Starcraft dir not found... maybe you should install it to C:/ first!"
  exit 1
fi

ln -s ~/TorchCraft/ $STARCRAFT_DIR/TorchCraft
mkdir -p $STARCRAFT_DIR/characters
cp ~/bwapi.?pc $STARCRAFT_DIR/characters

cd $STARCRAFT_DIR
bash TorchCraft/quick_setup.sh

winetricks -q nocrashdialog

# For some reason docker hates setting up permission correctly :-/
echo starcraft | sudo -s ls -lah /root > /dev/null
