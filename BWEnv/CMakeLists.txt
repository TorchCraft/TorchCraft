cmake_minimum_required(VERSION 3.1)
project(BWEnv)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(ZMQ REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

if (MSVC)
  # Enables TorchCraft usage on SSCAIT.
  # The SSCAIT submission process doesn't let you put files anywhere that
  # would allow them to be dynamically linked (namely, the working directory)
  # The fix is to statically link them to bypass that process.
  set(CMAKE_EXE_LINKER_FLAGS "-static")
endif(MSVC)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

if (USE_MINI_OPENBWAPI)
  find_path(OPENBW_DIR bwgame.h mini-openbwapi
    PATHS
    ENV OPENBW_DIR
    ../../../openbw
    ../../openbw
    ../openbw
  )
  
  if (NOT OPENBW_DIR)
    message(FATAL_ERROR "Couldn't find openbw. Specify the path to it with OPENBW_DIR environment variable")
  endif()
  
  add_subdirectory(${OPENBW_DIR}/mini-openbwapi mini-openbwapi)
  
  set(BWAPI_INCLUDE_DIRS ${OPENBW_DIR}/mini-openbwapi)
  set(BWAPI_LIBRARIES mini-openbwapi)
else()
  find_package(BWAPI REQUIRED)
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/fbs
  # XXX We shouldn't include this, but the includes would be a bit messed up
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/torchcraft
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${BWAPI_INCLUDE_DIRS}
)

add_library(BWEnvObj OBJECT
  src/config_manager.cc
  src/controller.cc
  src/user_actions.cc
  src/utils.cc
  src/zmq_server.cc
  ../replayer/frame.cpp
  ../replayer/frame_serialization.cpp
  ../replayer/frame_diff.cpp
  ../replayer/frame_diff_serialization.cpp
)

set_property(TARGET BWEnvObj PROPERTY POSITION_INDEPENDENT_CODE ON)

if (NOT USE_MINI_OPENBWAPI)
  add_library(BWEnv SHARED
    src/dll.cc
    src/module.cc
    $<TARGET_OBJECTS:BWEnvObj>
  )
  
  target_link_libraries(BWEnv
    ${ZMQ_LIBRARIES}
    ${BWAPI_LIBRARIES}
  )
endif()

add_executable(BWEnvClient
  src/main.cc
  $<TARGET_OBJECTS:BWEnvObj>
)

target_link_libraries(BWEnvClient
  ${ZMQ_LIBRARIES}
  ${BWAPI_LIBRARIES}
)
