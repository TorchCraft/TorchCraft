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

#include "../fbs/flatbuffers/flatbuffers.h"

namespace torchcraft {

  // In/OutStreamableFlatBuffer
  // Wrapper classes to Let us read/write Flatbuffer types to/from a stream.
  //
  // We need these because Flatbuffers don't have a fixed size;
  // they just read from their input pointer until complete,
  // an don't record the number of bytes consumed in the process.
  //
  // So you can't just read in a Flatbuffer from a stream, because you need
  // to read a chunk the stream in advance; and the amount of stream you need
  // to read is dynamic; so StreamableFlatbufferWrapper reads/writes metadata
  // (the size of the stored Flatbuffer) to enable streaming.
  //
  // A StreamableFlatbufferWrapper serializes to :
  //  {
  //    size_t  The flatbuffer's size,
  //    t       The flatbuffer
  //  }

  class OutStreamableFlatBuffer {

    public:
      flatbuffers::FlatBufferBuilder& flatBufferBuilder;
      OutStreamableFlatBuffer(
        flatbuffers::FlatBufferBuilder& finishedFlatBufferBuilder)
        : flatBufferBuilder(finishedFlatBufferBuilder) {
          // TODO: Verify that it's actually finished.
          // The actual .finished property is protected.
          // How can we tell?
        }

      void write(std::ostream& out) const {
        auto flatbufferPointer = flatBufferBuilder.GetBufferPointer();
        auto flatbufferSize = flatBufferBuilder.GetSize();
        out << flatbufferSize;
        out.write(
          reinterpret_cast<char const*>(flatbufferPointer),
          flatbufferSize);
      }
  };

  template <typename T>
  class InStreamableFlatBuffer{

    static_assert(
      std::is_base_of<flatbuffers::Table, T>::value,
      "Should be a FlatBuffer table.");

    public:
      typedef std::function<bool (const flatbuffers::Verifier&)> tInvokeVerifier;
      typedef std::function<T* (uint8_t*)> tInvokeReader;

    private:
      tInvokeVerifier invokeVerifier;
      tInvokeReader invokeReader;

    public:
      std::shared_ptr<T> flatbuffer;
      InStreamableFlatBuffer(
        tInvokeVerifier toVerify,
        tInvokeReader toRead):
        invokeVerifier(toVerify),
        invokeReader(toRead) {}

      void read(std::istream& in) {
        size_t bufferSize;
        in >> bufferSize;

        uint8_t buffer[bufferSize];
        in.read(buffer, bufferSize);

        flatbuffers::Verifier verifier(buffer, bufferSize);
        if ( ! invokeVerifier(verifier)) {
          throw std::runtime_error("Streaming FlatBuffer table failed verification");
        }

        flatbuffer = std::make_shared<T>(invokeReader(buffer));
      }
  };

  template <typename T>
  std::ostream& operator<<(std::ostream& oStream, const OutStreamableFlatBuffer& streamableFlatBuffer) {
    streamableFlatBuffer.write(oStream);
    return oStream;
  }

  template <typename T>
  std::istream& operator>>(std::istream& iStream, InStreamableFlatBuffer<T>& streamableFlatBuffer) {
    streamableFlatBuffer.read(iStream);
    return iStream;
  }
}
