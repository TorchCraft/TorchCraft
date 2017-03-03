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

  if (o.keyframe != 0) out << 0 << " " << o.keyframe << " ";
  out << height << " " << width << " ";
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      out << data[y * width + x] << " ";
    }
  }

  auto kf = o.keyframe == 0 ? 1 : o.keyframe;
  out << o.frames.size() << " ";
  for (size_t i = 0; i < o.frames.size(); i++) {
    if (i % kf == 0) out << *o.frames[i] << " ";
    else out << replayer::frame_diff(o.frames[i], o.frames[i - 1]) << " ";
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

  int32_t diffed;
  int32_t height, width;
  in >> diffed;

  if (diffed == 0) in >> o.keyframe >> height >> width;
  else {
    height = diffed;
    in >> width;
    o.keyframe = 0;
  }
  diffed = (diffed == 0); // Every kf is a Frame, others are frame diffs
  if (height <= 0 || width <= 0)
    throw std::runtime_error("Corrupted replay: invalid map size");
  uint8_t* data = (uint8_t*)THAlloc(sizeof(uint8_t) * height * width);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      in >> data[y * width + x];
    }
  }
  o.setMap(height, width, data);
  size_t nFrames;
  in >> nFrames;
  o.frames.resize(nFrames);
  for (size_t i = 0; i < nFrames; i++) {
    if (o.keyframe == 0) {
      o.frames[i] = new Frame();
      in >> *o.frames[i];
    }
    else {
      if (i % o.keyframe == 0) {
        o.frames[i] = new Frame();
        in >> *o.frames[i];
      }
      else {
        replayer::FrameDiff du;
        in >> du;
        o.frames[i] = replayer::frame_undiff(&du, o.frames[i-1]);
      }
    }
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
