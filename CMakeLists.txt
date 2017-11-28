CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)
ENABLE_TESTING()

# This CMake file is used _only_ really to install torchcraft in lua
IF(LUAROCKS_PREFIX)
    MESSAGE(STATUS "Installing TorchCraft through Luarocks")
    STRING(REGEX REPLACE "(.*)lib/luarocks/rocks.*" "\\1" CMAKE_INSTALL_PREFIX  "${LUAROCKS_PREFIX}")
    MESSAGE(STATUS "Prefix inferred from Luarocks: ${CMAKE_INSTALL_PREFIX}")
ENDIF()
IF(NOT MSVC)
    ADD_SUBDIRECTORY(test)
ENDIF(NOT MSVC)
FIND_PACKAGE(Torch REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(ZSTD libzstd>=1.0)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
SET(CMAKE_CXX_FLAGS "-Wall -fPIC -O2")

IF(ZSTD_FOUND)
    ADD_DEFINITIONS(-DWITH_ZSTD)
ENDIF()

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BWEnv/fbs
    ${CMAKE_CURRENT_SOURCE_DIR}/lua
    ${CMAKE_CURRENT_SOURCE_DIR}/replayer
    ${CMAKE_CURRENT_SOURCE_DIR}/client)

# A pure (non-Lua) C++ library containing the main client functionality
FILE(GLOB headers include/*.h)
FILE(GLOB cppsrc replayer/*.cpp client/*.cpp)
ADD_LIBRARY(torchcraft SHARED ${cppsrc})
IF(APPLE)
   SET_TARGET_PROPERTIES(torchcraft PROPERTIES
       LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()
SET_TARGET_PROPERTIES(torchcraft PROPERTIES
    VERSION   1.3.3
    SOVERSION 1.3.3)
TARGET_LINK_LIBRARIES(torchcraft luaT zmq)
IF(ZSTD_FOUND)
    # Use the static library since we're using "advanced/experimental" features
    TARGET_LINK_LIBRARIES(torchcraft ${ZSTD_LIBDIR}/libzstd.a)
ENDIF()
INSTALL(FILES ${headers}
        DESTINATION "${Torch_INSTALL_INCLUDE}/torchcraft"
)
INSTALL(TARGETS torchcraft
    LIBRARY DESTINATION ${Torch_INSTALL_LIB})

# The lua module, including a lua wrapper for the general client library
FILE(GLOB luasrc *.lua)
FILE(GLOB cppsrc_lua lua/*.cpp)
ADD_LIBRARY(tc_lib MODULE ${cppsrc_lua})
SET_TARGET_PROPERTIES(tc_lib PROPERTIES
   PREFIX ""
   IMPORT_PREFIX "")
IF(APPLE)
   SET_TARGET_PROPERTIES(tc_lib PROPERTIES
       LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()
TARGET_LINK_LIBRARIES(tc_lib TH luaT torchcraft)
INSTALL(TARGETS tc_lib
        DESTINATION "${LIBDIR}/torchcraft"
)

ADD_TORCH_PACKAGE(torchcraft "" "${luasrc}" "TorchCraft")
INSTALL(FILES DESTINATION "${Torch_INSTALL_LUA_PATH_SUBDIR}/torchcraft")
