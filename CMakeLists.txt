CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)
ENABLE_TESTING()

IF(LUAROCKS_PREFIX)
    MESSAGE(STATUS "Installing TorchCraft through Luarocks")
    STRING(REGEX REPLACE "(.*)lib/luarocks/rocks.*" "\\1" CMAKE_INSTALL_PREFIX  "${LUAROCKS_PREFIX}")
    MESSAGE(STATUS "Prefix inferred from Luarocks: ${CMAKE_INSTALL_PREFIX}")
   FIND_PACKAGE(Torch REQUIRED)
ENDIF()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE(ZMQ REQUIRED)
FIND_PACKAGE(ZSTD 1.0 COMPONENTS static)
IF(MSVC)
   ADD_DEFINITIONS(-DZMQ_STATIC)
ELSE(MSVC)
   SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
   SET(CMAKE_CXX_FLAGS "-Wall -fPIC -O2")
ENDIF(MSVC)

ADD_SUBDIRECTORY(test)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(ZSTD_FOUND)
    ADD_DEFINITIONS(-DWITH_ZSTD)
ENDIF()

FILE(GLOB headers include/torchcraft/*.h)
FILE(GLOB cppsrc replayer/*.cpp client/*.cpp)
ADD_LIBRARY(torchcraft SHARED ${cppsrc})
TARGET_INCLUDE_DIRECTORIES(torchcraft
   PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BWEnv/fbs
    ${CMAKE_CURRENT_SOURCE_DIR}/lua
    ${CMAKE_CURRENT_SOURCE_DIR}/replayer
    ${CMAKE_CURRENT_SOURCE_DIR}/client
   PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/torchcraft
    "${ZSTD_INCLUDE_DIR}"
    "${ZMQ_INCLUDE_DIR}"
 )

# https://github.com/zeromq/libzmq/issues/2884
IF(MSVC)
   IF(HAVE_WS2_32)
      TARGET_LINK_LIBRARIES(torchcraft ws2_32)
   ELSEIF(HAVE_WS2)
      TARGET_LINK_LIBRARIES(torchcraft ws2)
   ENDIF(HAVE_WS2_32)
ENDIF(MSVC)

# A pure (non-Lua) C++ library containing the main client functionality
IF(APPLE)
   SET_TARGET_PROPERTIES(torchcraft PROPERTIES
       LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()
SET_TARGET_PROPERTIES(torchcraft PROPERTIES
    VERSION   1.4.0
    SOVERSION 1.4.0
    PUBLIC_HEADER "${headers}")
TARGET_LINK_LIBRARIES(torchcraft ${ZMQ_LIBRARIES})
IF(ZSTD_FOUND)
    # Use the static library since we're using "advanced/experimental" features
    TARGET_LINK_LIBRARIES(torchcraft ${ZSTD_LIBRARY})
ENDIF()

IF(LUAROCKS_PREFIX)
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
ELSEIF(NOT MSVC)
   INSTALL(TARGETS torchcraft
      LIBRARY DESTINATION lib
      PUBLIC_HEADER DESTINATION include)
ENDIF(LUAROCKS_PREFIX)
