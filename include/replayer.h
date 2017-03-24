/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

extern "C" {
#include <TH/TH.h>
}

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

#include "frame.h"
#include "refcount.h"

namespace torchcraft {
namespace replayer {

struct Map {
  THByteTensor* data;
  Map() {
    data = nullptr;
  }
  ~Map() {
    if (data != nullptr)
      THByteTensor_free(data);
  }
};

class Replayer : public RefCounted {
 private:
  std::vector<Frame*> frames;
  std::unordered_map<int32_t, int32_t> numUnits;
  Map map;
  // If keyframe = 0, every frame is a frame.
  // Otherwise, every keyframe is a frame, and all others are diffs.
  // Only affects saving/loading (replays are still large in memory)
  uint32_t keyframe;

 public:
  ~Replayer() {
    for (auto f : frames) {
      if (f)
        f->decref();
    }
  }

  Frame* getFrame(size_t i) {
    if (i >= frames.size())
      return nullptr;
    return frames[i];
  }
  void push(Frame* f) {
    f->incref();
    frames.push_back(f);
  }
  void setKeyFrame(int32_t x) {
    keyframe = x < 0 ? frames.size() + 1 : (uint32_t)x;
  }
  uint32_t getKeyFrame() {
    return keyframe;
  }
  size_t size() const {
    return frames.size();
  }

  void setNumUnits() {
    for (const auto f : frames) {
      for (auto u : f->units) {
        auto s = u.second.size();
        auto i = u.first;
        if (numUnits.count(i) == 0) {
          numUnits[i] = s;
        } else if (s > numUnits[i]) {
          numUnits[i] = s;
        }
      }
    }
  }

  int32_t getNumUnits(const int32_t& key) const {
    if (numUnits.find(key) == numUnits.end())
      return -1;
    return numUnits.at(key);
  }

  void setMap(THByteTensor* walkability, THByteTensor* ground_height,
      THByteTensor* buildability,
      std::vector<int>& start_loc_x, std::vector<int>& start_loc_y);

  void setMap(int32_t h, int32_t w,
      uint8_t* walkability, uint8_t* ground_height, uint8_t* buildability,
      std::vector<int>& start_loc_x, std::vector<int>& start_loc_y);

  void setRawMap(uint32_t w, uint32_t h, uint8_t* d) {
    // free existing map if needed
    if (map.data != nullptr) {
      THByteTensor_free(map.data);
    }
    auto storage = THByteStorage_newWithData(d, h * w); // refcount 1
    map.data = THByteTensor_newWithStorage2d(storage, 0, w, h, h, 1);
    // storage has been retained by map.data, so decrease refcount back to 1
    THByteStorage_free(storage);
  }

  THByteTensor* getRawMap() {
    return map.data;
  }

  void getMap(THByteTensor* walkability, THByteTensor* ground_height,
      THByteTensor* buildability,
      std::vector<int>& start_loc_x, std::vector<int>& start_loc_y);

  friend std::ostream& operator<<(std::ostream& out, const Replayer& o);
  friend std::istream& operator>>(std::istream& in, Replayer& o);
};

} // namespace replayer
} // namespace torchcraft
