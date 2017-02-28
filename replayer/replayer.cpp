/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "replayer.h"

namespace replayer = torchcraft::replayer;

// Serialization

std::ostream& replayer::operator<<(
    std::ostream& out,
    const replayer::Replayer& o) {
  auto height = THByteTensor_size(o.map.data, 0);
  auto width = THByteTensor_size(o.map.data, 1);
  auto data = THByteTensor_data(o.map.data);

  out << height << " " << width << " ";
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      out << data[y * width + x] << " ";
    }
  }

  out << o.frames.size() << " ";
  for (auto& f : o.frames) {
    out << *f << " ";
  }

  out << o.numUnits.size() << " ";
  for (const auto& nu : o.numUnits) {
    out << nu.first << " " << nu.second << " ";
  }

  return out;
}

std::istream& replayer::operator>>(std::istream& in, replayer::Replayer& o) {
  // WARNING: cases were observed where this operator left a Replayer
  // that was in a corrupted state, and would produce a segfault
  // if we tried to delete it.
  // Cause: invalid data file? I/O error? or a bug in the code?

  int height, width;
  in >> height >> width;
  if (height <= 0 || width <= 0)
    throw std::runtime_error("Corrupted replay: invalid map size");
  uint8_t* data = (uint8_t*)THAlloc(sizeof(uint8_t) * height * width);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      in >> data[y * width + x];
    }
  }
  o.setMap(height, width, data);
  int nFrames;
  in >> nFrames;
  if (nFrames < 0)
    throw std::runtime_error("Corrupted replay: nFrames < 0");
  o.frames.resize(nFrames);
  for (size_t i = 0; i < nFrames; i++) {
    o.frames[i] = new Frame();
    in >> *o.frames[i];
  }

  int s;
  in >> s;
  if (s < 0)
    throw std::runtime_error("Corrupted replay: s < 0");
  int32_t key, val;
  for (auto i = 0; i < s; i++) {
    in >> key >> val;
    o.numUnits[key] = val;
  }

  return in;
}
