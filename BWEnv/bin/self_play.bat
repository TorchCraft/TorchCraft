cd /d %~dp0
start ./bwheadless.exe -e StarCraft.exe -l bwapi-data/BWAPI.dll --headful
echo "Please wait..."
TIMEOUT 2
start ./bwheadless.exe -e StarCraft.exe -l bwapi-data/BWAPI.dll --headful