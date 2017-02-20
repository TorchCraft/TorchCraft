/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

extern "C" {
#include <TH/TH.h>
#include <lua.h>
#include <luaT.h>
#include <lauxlib.h>
#include <lualib.h>
}

extern "C" int rawBitmapToTensor(lua_State* L);
