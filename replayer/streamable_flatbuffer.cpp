/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>
#include <memory>
#include <type_traits>

#include "streamable_flatbuffer.h"

namespace torchcraft {

  void writeFlatBufferToStream(std::ostream& out, flatbuffers::FlatBufferBuilder& finishedFlatBufferBuilder) {

    // Assert that the FlatBuffer is actually finished.
    // This is an internal FlatBuffers API call,
    // but they don't expose this information any other way.
    finishedFlatBufferBuilder.Finished();

    auto flatbufferPointer = finishedFlatBufferBuilder.GetBufferPointer();
    size_t flatbufferSize = finishedFlatBufferBuilder.GetSize();
    out.write(reinterpret_cast<char*>(&flatbufferSize), sizeof(size_t));
    out.write(
      reinterpret_cast<char*>(flatbufferPointer),
      flatbufferSize);
  }
}
