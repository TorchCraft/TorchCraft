/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "imgmanager.h"

// Take raw string data and push a 3D ByteTensor to the Lua stack.
extern "C" int rawBitmapToTensor(lua_State* L) {
  auto str = luaL_checkstring(L, 1);

  auto rows = luaL_checkint(L, 2);
  auto cols = luaL_checkint(L, 3);

  // HACK HACK HACK :(
  // FIXME put data on heap, make pointer operation faster / parallel

  int matrix_size = 3 * rows * cols;
  unsigned char rgb_data[matrix_size];

  // Incoming binary data is [BGRA,...], which we transform into [R..,G..,B..].
  int k = 0;
  for (int a = 2; a >= 0; --a)
    {
      int it = a;
      for (int i = 0; i < matrix_size / 3; i++)
        {
          rgb_data[k] = str[it];
          it += 4;
          ++k;
        }
    }

  auto storage = THByteStorage_newWithData(rgb_data, matrix_size);
  THByteTensor* image_th = THByteTensor_newWithStorage3d(storage,
                                                         0, 3,
                                                         rows * cols,
                                                         cols, rows,
                                                         rows, 1);

  luaT_pushudata(L, (void*)image_th, "torch.ByteTensor");
  return 1;
}
