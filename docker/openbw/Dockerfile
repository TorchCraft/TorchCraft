FROM nvidia/cuda:8.0-cudnn7-devel-ubuntu16.04
MAINTAINER Nantas Nardelli

# -------- build args
ARG TC_BRANCH=master
ARG TC_PORT=11111
ARG OBW_GUI=1

# -------- CUDA env
ENV CUDA_PATH /usr/local/cuda
ENV CUDA_INCLUDE_PATH /usr/local/cuda/include
ENV CUDA_LIBRARY_PATH /usr/local/cuda/lib64

# -------- Ubuntu Packages
RUN apt-get update -y && apt-get install software-properties-common -y && \
    add-apt-repository -y multiverse && apt-get update -y && apt-get upgrade -y && \
    apt-get install -y apt-utils git nano vim man build-essential wget sudo && \
    rm -rf /var/lib/apt/lists/*
RUN apt-get update

# -------- OpenBW

## --------- compilation toolset
RUN apt-get install -y libtool pkg-config build-essential autoconf automake uuid-dev cmake

## --------- sdl
RUN apt-get install -y libsdl2-dev libsdl2-2.0

## --------- build OpenBW
RUN git clone https://github.com/openbw/openbw
RUN git clone https://github.com/openbw/bwapi
RUN cd bwapi && mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DOPENBW_DIR=../../openbw -DOPENBW_ENABLE_UI=$OBW_GUI \
    && make && make install

# -------- TorchCraft

RUN git clone https://github.com/torchcraft/torchcraft.git -b $TC_BRANCH --recursive

## --------- zmq
RUN apt-get purge -y libzmq*
RUN apt-get install -y libtool pkg-config build-essential autoconf automake uuid-dev
RUN wget https://github.com/zeromq/libzmq/releases/download/v4.2.2/zeromq-4.2.2.tar.gz
RUN tar xvzf zeromq-4.2.2.tar.gz
# RUN ulimit -n 1000 && apt-get update
RUN cd zeromq-4.2.2 && ./configure && make install && ldconfig

## --------- zstd
# TODO check if we need to install zstd from source
RUN apt-get install -y libzstd1-dev zstd

## --------- build BWEnv
RUN cd torchcraft/BWEnv && mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=relwithdebinfo \
    && make -j

# Installing the python client just in case
## --------- python3
RUN apt-get update
RUN apt-get -y install python3
RUN apt-get -y install python3-pip
RUN pip3 install --upgrade pip

## --------- python client deps
RUN pip3 install pybind11

## --------- python client
RUN cd torchcraft && pip3 install .