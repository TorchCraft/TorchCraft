CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

FIND_PACKAGE(gflags REQUIRED)

IF(ZSTD_FOUND)
    ADD_DEFINITIONS(-DWITH_ZSTD)
ENDIF()

INCLUDE_DIRECTORIES(
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../../
   ${CMAKE_CURRENT_SOURCE_DIR}/../../include
   ${CMAKE_CURRENT_SOURCE_DIR}/../../replayer
   ${CMAKE_CURRENT_SOURCE_DIR}/../../client)

 FILE(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/../../include/*.h)
FILE(GLOB cppsrc 
   ${CMAKE_CURRENT_SOURCE_DIR}/../../replayer/*.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/../../client/*.cpp)
ADD_LIBRARY(torchcraft SHARED ${cppsrc})
ADD_CUSTOM_COMMAND(TARGET torchcraft PRE_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
   ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_BINARY_DIR}/torchcraft)
IF(APPLE)
   SET_TARGET_PROPERTIES(torchcraft PROPERTIES
       LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()
SET_TARGET_PROPERTIES(torchcraft PROPERTIES
    VERSION   1.3.3
    SOVERSION 1.3.3)
TARGET_LINK_LIBRARIES(torchcraft zmq)
IF(ZSTD_FOUND)
    # Use the static library since we're using "advanced/experimental" features
    TARGET_LINK_LIBRARIES(torchcraft ${ZSTD_LIBDIR}/libzstd.a)
ENDIF()
INSTALL(FILES ${headers} DESTINATION "${CMAKE_ROOT}/torchcraft")

ADD_EXECUTABLE(simple_dll simple_dll.cpp)
TARGET_LINK_LIBRARIES(simple_dll torchcraft gflags)
