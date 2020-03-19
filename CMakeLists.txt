#Not in 2
OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)

#3.10 in 2
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

#Not in 2
ENABLE_TESTING()

# Similar to 2
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#FindZMQ.cmake is identical in TC2 but not referenced anywhere
FIND_PACKAGE(ZMQ REQUIRED)

#FindZSTD.cmake is identical in TC2
# Done differently in 2
FIND_PACKAGE(ZSTD 1.0 COMPONENTS static)

#ZMQ not referenced in TC2
IF(MSVC)
   ADD_DEFINITIONS(-DZMQ_STATIC)
ELSE(MSVC)
   SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
   SET(CMAKE_CXX_FLAGS "-Wall -fPIC -O2")
ENDIF(MSVC)

ADD_SUBDIRECTORY(test)

# This was commented out before
# Linking libtorchcraft fails due to: 
# > /usr/bin/ld: cannot find -lBWAPILIB
ADD_SUBDIRECTORY(thirdparty/bwapilib)

# Same in 2
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2 isn't so careful
IF(ZSTD_FOUND)
   ADD_DEFINITIONS(-DWITH_ZSTD)
ENDIF()

############## Everything below is TC1 ##############
FILE(GLOB headers include/torchcraft/*.h)
FILE(GLOB cppsrc replayer/*.cpp client/*.cpp)
ADD_LIBRARY(torchcraft ${cppsrc})
TARGET_INCLUDE_DIRECTORIES(torchcraft
   PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BWEnv/fbs
    ${CMAKE_CURRENT_SOURCE_DIR}/replayer
    ${CMAKE_CURRENT_SOURCE_DIR}/client
   PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/torchcraft
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/bwapilib
    "${ZSTD_INCLUDE_DIR}"
    "${ZMQ_INCLUDE_DIR}"
 )

IF(MSVC)
   TARGET_LINK_LIBRARIES(torchcraft shlwapi ws2_32 wsock32 Iphlpapi)
ELSEIF(APPLE)
   SET_TARGET_PROPERTIES(torchcraft PROPERTIES
       LINK_FLAGS "-undefined dynamic_lookup")
ENDIF(MSVC)

SET_TARGET_PROPERTIES(torchcraft PROPERTIES
    VERSION   2.0.0
    SOVERSION 2.0.0
    PUBLIC_HEADER "${headers}")
TARGET_LINK_LIBRARIES(torchcraft ${ZMQ_LIBRARIES} BWAPILIB)
IF(ZSTD_FOUND)
    # Use the static library since we're using "advanced/experimental" features
    TARGET_LINK_LIBRARIES(torchcraft ${ZSTD_LIBRARY})
ENDIF()

IF(NOT MSVC)
   INSTALL(TARGETS torchcraft
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      PUBLIC_HEADER DESTINATION include)
ENDIF(NOT MSVC)
