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

  // Lets us read/write Flatbuffer types to/from a stream.
  //
  // We need this because Flatbuffers don't have a fixed size;
  // they just read from their input pointer until complete,
  // an don't record the number of bytes consumed in the process.
  //
  // So you can't just read in a Flatbuffer from a stream, because you need
  // to read a chunk the stream in advance; and the amount of stream you need
  // to read is dynamic; so StreamableFlatbufferWrapper reads/writes metadata
  // (the size of the stored Flatbuffer) to enable streaming.

  template <typename T>
  class AbstractFlatbufferWrapper {

    static_assert(
      std::is_default_constructible<T>::value,
      "Should be a default-constructible FlatBuffer (but isn't default-constructible)");
    static_assert(
      std::is_base_of<flatbuffers::Table, T>::value ||
      std::is_base_of<flatbuffers::Struct, T>::value,
      "Should be a default-constructible FlatBuffer (but isn't a Flatbuffer)");

    protected:
      AbstractFlatbufferWrapper() {}
  };

  template <typename T>
  class OutStreamableFlatbuffer : protected AbstractFlatbufferWrapper<T> {

    public:
      T& flatbuffer;
      OutStreamableFlatbuffer(T& unfinishedFlatbuffer)
        : flatbuffer(unfinishedFlatbuffer) {
        unfinishedFlatbuffer.Finish();
      }

      void write(std::ostream& out) {
        flatbuffer->Finish();
        auto flatbufferPointer = flatbuffer->GetBufferPointer();
        auto flatbufferSize = flatbuffer.GetSize();

        out << flatbufferSize;
        out.write(reinterpret_cast<char const*>(flatbufferPointer), flatbufferSize);
      }
  };

  template <typename T>
  class InStreamableFlatbuffer : protected AbstractFlatbufferWrapper<T> {

    public:
      std::shared_ptr<T> flatbuffer;
      InStreamableFlatbuffer() {}
  };

  // A StreamableFlatbufferWrapper serializes to :
  //  {
  //    size_t  The flatbuffer's size,
  //    t       The flatbuffer
  //  }

  template <typename T>
  std::ostream& operator<<(std::ostream& out, const OutStreamableFlatbuffer<T>& o) {
    out.write(out, o);
    return out;
  }

  template <typename T>
  std::istream& operator>>(std::istream& in, InStreamableFlatbuffer<T>& o) {

    size_t bufferSize;
    in >> bufferSize;

    o.getFlatbuffer()->data.resize(bufferSize);
    in.read(o.getFlatbuffer()->data.data(), bufferSize);

    // TODO: Use verifier?
    // flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(content->data.data()), content->data.size());
    // if (!fbs::VerifyReducedUnitTypesBuffer(verifier)) {
    //  throw std::runtime_error("corrupted data");
    //}

    // TODO: How to get the actual object? Or is that not necessary
    // red->d = ffbs::GetReducedUnitTypes(red->data.data());

    return in;
  }
}
