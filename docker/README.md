# Torchcraft on Docker

## OpenBW image

You can build the image by running:
```bash
$ cd docker
$ docker build -f openbw/Dockerfile -t torchcraft-obw --build-arg TC_BRANCH=master OBW_GUI=1
```

Then you can simply run BWAPILauncher by doing:

```bash
$ docker run -it -p 11111:11111 -v /your/path/to/mpqs:/mpqs torchcraft-obw
root@image$ cd /mpqs
root@image$ OPENBW_ENABLE_UI=0 \
   BWAPI_CONFIG_AI__RACE=Terran \
   BWAPI_CONFIG_AI__AI=/torchcraft/BWEnv/build/BWEnv.so \
   BWAPI_CONFIG_AUTO_MENU__AUTO_MENU="SINGLE_PLAYER" \
   BWAPI_CONFIG_AUTO_MENU__MAP=/torchcraft/maps/micro/m5v5_c_far.scm \
   BWAPI_CONFIG_AUTO_MENU__GAME_TYPE="USE_MAP_SETTINGS" \
   TORCHCRAFT_PORT=11111 \
   BWAPILauncher
```

* `TC_BRANCH` allows you to control which version of torchcraft to clone (defaults to `master`)
* `OBW_GUI` allows you to control whether to compile OpenBW with sdl support (defaults to `1`)


## Standard Image

### Building the image

First, download the [Starcraft: Brood War installer](https://us.battle.net/account/management/)
to `TorchCraft/docker/common/Downloader_StarCraft_Combo_enUS.exe`

From the current directory:

`docker build -f no-cuda/Dockerfile -t torchcraft .`

or if you want CUDA, install the [nvidia docker](https://github.com/NVIDIA/nvidia-docker) plugin first and:

`docker build -f cuda/Dockerfile -t cutorchcraft .`

To run the client:

```
docker run --rm --privileged -it -p 5900:5900 torchcraft bash
# For a docker image with CUDA support:
nvidia-docker run --rm --privileged -it -p 5900:5900 cutorchcraft bash
# Setup wine
wine wineboot --init
winetricks -q vcrun2013
```

Use your favorite VNC client to connect to the docker image. For example, with
TigerVNC, do `vncviewer localhost:0`. The password is `mot2pass` for the VNC
connection.

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


The sudo password to the image is `starcraft`

## Saving and reloading
This is a good time to [save a copy of your image](http://stackoverflow.com/questions/24482822/how-to-share-my-docker-image-without-using-the-docker-hub) in case it gets corrupted, so you don't have to go through that process again.

### Exporting
```
# Note the ID of the docker container you just made
docker ps -a
docker commit $ID
# Note the ID of the image you just created
# Optionally, tag it with something else
docker tag $IMAGE_ID torchcraft:my_version
docker save $IMAGE_ID -o torchcraft.docker
```

### Importing
```
docker load -i torchcraft.docker
docker run --rm --privileged -it -p 5900:5900 torchcraft:my_version bash
```
