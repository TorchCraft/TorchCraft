cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(OPENBW_DIR bwgame.h mini-openbwapi
  PATHS
  ENV OPENBW_DIR
  ../../../tsc-bw
  ../../tsc-bw
  ../tsc-bw
)

if (NOT OPENBW_DIR)
  message(FATAL_ERROR "Couldn't find openbw. Specify the path to it with OPENBW_DIR environment variable (git clone https://github.com/tscmoo/tsc-bw)")
endif()

add_subdirectory(${OPENBW_DIR}/mini-openbwapi mini-openbwapi)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/fbs
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${OPENBW_DIR}/mini-openbwapi
)

add_executable(bwenv
  src/main.cc
  src/config_manager.cc
  src/controller.cc
  src/user_actions.cc
  src/utils.cc
  src/zmq_server.cc
  ../replayer/frame.cpp
)

target_link_libraries(bwenv
  czmq
  mini-openbwapi
)
