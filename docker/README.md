# Torchcraft on Docker
## Install the image

First, download the [Starcraft: Brood War installer](https://us.battle.net/account/management/)
to `TorchCraft/docker/common/Downloader_StarCraft_Combo_enUS.exe`

From the current directory:

`docker build -f no-cuda/Dockerfile -t torchcraft .`

or if you want CUDA (TODO):

`docker build -f cuda/Dockerfile -t torchcraft .`

To run the client:

```
# Start your VNC Server. This will run as a daemon in the background
docker run -d --name display -e VNC_PASSWORD=newPW -p 5900:5900 suchja/x11server
# Run the Torchcraft docker image
docker run --rm --privileged -it --link display:xserver --volumes-from display torchcraft /bin/bash
# Setup wine
wine wineboot --init
winetricks -q vcrun2013
```

Use your favorite VNC client to connect to the docker image. For example, with
TigerVNC, do `vncviewer localhost:0`. If the daemon of suchja/x11server is ever
closed, you must find the id with `docker ps -a`, remove it with `docker rm $ID`
and restart the server with the above command.

## Install StarCraft

0. In your docker container...
1. `wine Downloader_StarCraft_Combo_enUS.exe`
2. Follow the instructions in the VNC server.
3. **INSTALL STARCRAFT INTO C:\StarCraft**
4. `wine BW-1161.exe` and follow the instructions in the GUI
5. `wine BWAPI_412_Setup.exe` and follow the instructions in the GUI.
   **INSTALL INTO C:\StarCraft\BWAPI**
6. `bash ~/quick_start.sh`
7. `cd ~/.wine/drive_c/StarCraft`
8. Launch Starcraft! `winegui bwheadless.exe -e StarCraft.exe -l bwapi-data/BWAPI.dll --headful`
9. Use `docker ps` to find the container id of the torchcraft container, and run `docker exec -it $ID bash`
   to jump into the container from a different terminal. From there, you can do things like 
   `cd ~/TorchCraft/examples && th simple_dll.lua -t localhost`
   

The sudo password to the image is 'starcraft'
   
## Saving and reloading
This is a good time to [save a copy of your image](http://stackoverflow.com/questions/24482822/how-to-share-my-docker-image-without-using-the-docker-hub) in case it gets corrupted, so you don't have to go through that process again.

### Exporting
```
# Note the ID of the docker container you just made
docker ps -a
docker export $ID -o torchcraft.docker
```

### Importing
```
docker import torchcraft.docker
# Note the output ID, which can also be found with `docker images`
docker run --user torchcraft --rm --privileged -it --link display:xserver --volumes-from display $ID bash
```
