CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)
IF(LUAROCKS_PREFIX)
    MESSAGE(STATUS "Installing TorchCraft through Luarocks")
    STRING(REGEX REPLACE "(.*)lib/luarocks/rocks.*" "\\1" CMAKE_INSTALL_PREFIX  "${LUAROCKS_PREFIX}")
    MESSAGE(STATUS "Prefix inferred from Luarocks: ${CMAKE_INSTALL_PREFIX}")
ENDIF()
FIND_PACKAGE(Torch REQUIRED)
#SET(BUILD_STATIC yes)

FILE(GLOB luasrc *.lua)
SET(CMAKE_CXX_FLAGS -std=c++11)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/replayer
    ${CMAKE_CURRENT_SOURCE_DIR}/client)

ADD_LIBRARY(shared STATIC
    replayer/frame.cpp
    replayer/gamestore.cpp
    replayer/imgmanager.cpp
    replayer/replayer.cpp)

ADD_LIBRARY(replayer MODULE
    replayer/frame_lua.cpp
    replayer/init.cpp)
SET_TARGET_PROPERTIES(replayer PROPERTIES
   PREFIX ""
   IMPORT_PREFIX "")
IF(APPLE)
   SET_TARGET_PROPERTIES(replayer PROPERTIES
       LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()
TARGET_LINK_LIBRARIES(replayer shared TH luaT)

FILE(GLOB client_src client/*.cpp)
ADD_LIBRARY(client MODULE ${client_src})
SET_TARGET_PROPERTIES(client PROPERTIES
   PREFIX ""
   IMPORT_PREFIX "")
IF(APPLE)
   SET_TARGET_PROPERTIES(client PROPERTIES
       LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()
TARGET_LINK_LIBRARIES(client shared TH luaT zmq)

INSTALL(TARGETS replayer client
        DESTINATION "${LIBDIR}/torchcraft"
)


ADD_TORCH_PACKAGE(torchcraft "" "${luasrc}" "TorchCraft")
INSTALL(FILES "starcraft_static.json" DESTINATION "${Torch_INSTALL_LUA_PATH_SUBDIR}/torchcraft")
