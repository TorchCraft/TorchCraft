/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#define WIN32_LEAN_AND_MEAN
#include <iostream>

#include "controller.h"

int main(int argc, const char* argv[])
{
  std::cout << "Welcome to the Brood War TorchCraft Environment." << std::endl;
  std::cout << "Compiled on "
    << __DATE__ << ", " << __TIME__ << "." << std::endl;

  try {
    Controller c(true);

    if (!c.connect_server())
      return 1;

    c.loop();
  } catch (const std::exception& e) {
    std::cout << "exception: " << e.what() << std::endl;
  }

  return 0;
}
