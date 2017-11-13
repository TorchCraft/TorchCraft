/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>
#include "lest/lest.hpp"
#include "frame.h"

const lest::test specification[] = {
  lest_CASE("A TorchCraft frame is invariant through serialization") {
    SETUP("Create a TorchCraft frame") {
      auto foo = 3;

      EXPECT(foo == 3);

      SECTION("Passing tests") {
        EXPECT(0 == 0);
        EXPECT(true);
      }
      SECTION("Failing tests") {
        EXPECT(0 == 1);
        EXPECT(false);
      }
    }
  },
};


// Run with "ctest test --output-on-failure"
int main(int argc, char* argv[]) {
  std::cout << "Running tests with console output" << std::endl;
  return lest::run(specification, argc, argv, std::cerr);
}
