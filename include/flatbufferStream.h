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

  template <typename T>
  class FlatbufferStream {

    static_assert(
      std::is_default_constructible<T>::value,
      "Should be a default-constructible FlatBuffer (but isn't default-constructible)");
    static_assert(
      std::is_base_of<flatbuffers::Table, T>::value ||
      std::is_base_of<flatbuffers::Struct, T>::value,
      "Should be a default-constructible FlatBuffer (but isn't a Flatbuffer)");

    public:
      FlatbufferStream() {}
      FlatbufferStream(const T& finishedFlatbuffer): flatbuffer(finishedFlatbuffer) {}
      std::shared_ptr<T> getFlatbuffer() { std::shared_ptr<T>(flatBuffer); }

    private:
      std::shared_ptr<T> flatbuffer;
  };

  // A FlatbufferStream serializes to :
  //  {
  //    size_t  The flatbuffer's size,
  //    t       The flatbuffer
  //  }
  //
  //  Flatbuffers are ignorant of how mcuh

  template <typename T>
  std::ostream& operator<<(std::ostream& out, const FlatbufferStream<T>& o) {
    return out;
  }

  template <typename T>
  std::istream& operator>>(std::istream& in, FlatbufferStream<T>& o) {

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
