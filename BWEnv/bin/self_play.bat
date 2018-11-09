:: Copyright (c) 2015-present, Facebook, Inc.
:: All rights reserved.
:: 
:: This source code is licensed under the BSD-style license found in the
:: LICENSE file in the root directory of this source tree. An additional grant
:: of patent rights can be found in the PATENTS file in the same directory.

cd /d %~dp0
start ./bwheadless.exe -e StarCraft.exe -l bwapi-data/BWAPI.dll --headful
echo "Please wait..."
TIMEOUT 2
start ./bwheadless.exe -e StarCraft.exe -l bwapi-data/BWAPI.dll --headful
