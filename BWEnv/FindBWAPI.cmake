# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
# 
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

# This defines
# BWAPI_FOUND - if BWAPI was found
# BWAPI_INCLUDE_DIRS - include directories
# BWAPI_LIBRARIES - libraries to link to

find_path(BWAPI_INCLUDE_DIR BWAPI.h PATHS ${BWAPI_DIR} ENV BWAPI_DIR PATH_SUFFIXES include)
find_library(BWAPI_LIBRARY BWAPILIB PATHS ${BWAPI_DIR} ENV BWAPI_DIR PATH_SUFFIXES lib)
find_library(BWAPIClient_LIBRARY BWAPIClient PATHS ${BWAPI_DIR} ENV BWAPI_DIR PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BWAPI DEFAULT_MSG BWAPI_LIBRARY BWAPI_INCLUDE_DIR)

mark_as_advanced(BWAPI_INCLUDE_DIR BWAPI_LIBRARY)

set(BWAPI_INCLUDE_DIRS ${BWAPI_INCLUDE_DIR})
set(BWAPI_LIBRARIES ${BWAPI_LIBRARY} ${BWAPIClient_LIBRARY})
