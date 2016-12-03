#!/bin/bash

tc_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ ! -f "StarCraft.exe" ]; then
	echo "Please execute this in the StarCraft directory (~/.wine/drive_c/StarCraft)"
	exit 1
fi
sc_dir=$(readlink -f .)

cp $tc_dir/BWEnv/bin/*.exe ./
cp $tc_dir/BWEnv/bin/*.dll ./
cp $tc_dir/BWEnv/bin/*.sh ./
cp $tc_dir/config/*.ini $sc_dir/bwapi-data/
cp $tc_dir/maps/micro/ $sc_dir/Maps/BroodWar/ -r
# curl -LO https://github.com/soumith/TorchCraft/releases/download/v0.4-alpha/BWEnv.dll
# curl -LO https://github.com/soumith/TorchCraft/releases/download/v0.4-alpha/BWEnv.exe
echo "WARNING: Since the repo is private, you'll have to download these files into your \$STARCRAFT folder yourself :<"
echo https://github.com/soumith/TorchCraft/releases/download/v0.4-alpha/BWEnv.dll
echo https://github.com/soumith/TorchCraft/releases/download/v0.4-alpha/BWEnv.exe


sed -i "s|^ai *=.*|ai=$sc_dir/BWEnv.dll|" bwapi-data/bwapi.ini

# Set up the right registry keys
cd $(pwd | grep -o '.*/.wine/')
cp user.reg user.reg.BAK  # Make a backup
cat $tc_dir/docker/common/regkeys >> user.reg
