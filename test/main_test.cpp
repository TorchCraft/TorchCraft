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
#include "flatbuffers.h"

namespace torchcraft {
namespace replayer {

  const lest::test specification[] = {
    lest_CASE("A TorchCraft frame is invariant through serialization") {
      SETUP("Create & serialize a frame") {
        torchcraft::replayer::Frame f0;
        torchcraft::replayer::Frame f1;

        flatbuffers::FlatBufferBuilder builder;
        f0.addToFlatBufferBuilder(builder);



        f0.reward += 1.23;
        f0.is_terminal = ! f0.is_terminal;

        std::stringstream buffer;
        buffer << f0;
        buffer >> f1;

        EXPECT(f0.reward == f1.reward);
        EXPECT(f0.is_terminal == f1.is_terminal);
        EXPECT(f0.creep_map == f1.creep_map);
      }
    },
  };
} // namespace torchcraft
} // namespace replayer


// For output, run either as:
// > ctest test --output-on-failure
// > make ARGS='-VV' test
int main(int argc, char* argv[]) {
  std::cout << "Running tests with console output" << std::endl;
  return lest::run(torchcraft::replayer::specification, argc, argv, std::cerr);
}
