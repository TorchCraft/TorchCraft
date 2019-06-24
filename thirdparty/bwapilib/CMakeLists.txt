IF(NOT MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
 -Wno-unknown-pragmas\
 -Wno-unused-variable\
 -Wno-unused-const-variable\
 -Wno-unused-but-set-variable\
 -Wno-int-to-pointer-cast\
 -Wno-int-to-void-pointer-cast\
 -Wno-unknown-warning-option")
ENDIF(NOT MSVC)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB SRCS *.cpp)
ADD_LIBRARY(bwapilib ${SRCS})
SET_PROPERTY(TARGET bwapilib PROPERTY POSITION_INDEPENDENT_CODE ON)
