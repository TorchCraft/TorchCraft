/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <iostream>
#include <memory>
#include <type_traits>

#include "flatbuffers.h"
#include "streamable_flatbuffer.h"

namespace torchcraft {

  // Methods to write/read Flatbuffer types to/from a stream.
  //
  // We need these because Flatbuffers don't have a fixed size;
  // they just read from their input pointer until complete,
  // an don't record the number of bytes consumed in the process.
  //
  // So you can't just read in a Flatbuffer from a stream, because you need
  // to read a chunk the stream in advance; and the amount of stream you need
  // to read is dynamic; so these methods reads/write metadata
  // (the size of the stored Flatbuffer) to enable streaming.
  //
  // Serialization format:
  //  {
  //    size_t  The flatbuffer's size,
  //    t       The flatbuffer
  //  }

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

  template <typename T>
  std::shared_ptr<const T> readFlatBufferTableFromStream(std::istream& in) {

    static_assert(
      std::is_base_of<flatbuffers::Table, T>::value,
      "Should be a FlatBuffer table.");

    size_t bufferSize;
    in.read(reinterpret_cast<char*>(&bufferSize), sizeof(size_t));

    char buffer[bufferSize];
    in.read(buffer, bufferSize);

    flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(buffer), bufferSize);
    if (verifier.VerifyBuffer<T>()) {
      throw std::runtime_error("Streaming FlatBuffer table failed verification");
    };

    auto table = flatbuffers::GetRoot<T>(buffer);
    return std::shared_ptr<const T>(table);
  };
}