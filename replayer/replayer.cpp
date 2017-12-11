/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "replayer.h"
#include <bitset>

#ifdef WITH_ZSTD
#include "zstdstream.h"
#endif

namespace torchcraft {
namespace replayer {

// Serialization

std::ostream& operator<<(std::ostream& out, const Replayer& o) {
  auto height = o.map.height;
  auto width = o.map.width;
  auto data = o.map.data.data();

  if (o.keyframe != 0)
    out << 0 << " " << o.keyframe << " ";
  out << height << " " << width << " ";
  out.write((const char*)data, height * width); // Write map data as raw bytes

  auto kf = o.keyframe == 0 ? 1 : o.keyframe;
  out << o.frames.size() << " ";
  for (size_t i = 0; i < o.frames.size(); i++) {
    if (i % kf == 0)
      out << *o.frames[i];
    else
      out << frame_diff(o.frames[i], o.frames[i - 1]);
  }

  out << o.numUnits.size() << " ";
  for (const auto& nu : o.numUnits) {
    out << nu.first << " " << nu.second << " ";
  }

  return out;
}

std::istream& operator>>(std::istream& in, Replayer& o) {
  // WARNING: cases were observed where this operator left a Replayer
  // that was in a corrupted state, and would produce a segfault
  // if we tried to delete it.
  // Cause: invalid data file? I/O error? or a bug in the code?

  int32_t diffed;
  int32_t height, width;
  in >> diffed;

  if (diffed == 0)
    in >> o.keyframe >> height >> width;
  else {
    height = diffed;
    in >> width;
    o.keyframe = 0;
  }
  diffed = (diffed == 0); // Every kf is a Frame, others are frame diffs
  if (height <= 0 || width <= 0 || height > 10000 || width > 10000)
    throw std::runtime_error("Corrupted replay: invalid map size");
  uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * height * width);
  in.ignore(1); // Ignores next space
  in.read((char*)data, height * width); // Read some raw bytes
  o.setRawMap(height, width, data);
  size_t nFrames;
  in >> nFrames;
  o.frames.resize(nFrames);
  in.ignore(1); // Ignores next space
  for (size_t i = 0; i < nFrames; i++) {
    if (o.keyframe == 0) {
      o.frames[i] = new Frame();
      in >> *o.frames[i];
    } else {
      if (i % o.keyframe == 0) {
        o.frames[i] = new Frame();
        in >> *o.frames[i];
      } else {
        FrameDiff du;
        in >> du;
        o.frames[i] = frame_undiff(&du, o.frames[i - 1]);
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

void Replayer::setMap(
    int32_t h,
    int32_t w,
    std::vector<uint8_t> const& walkability,
    std::vector<uint8_t> const& ground_height,
    std::vector<uint8_t> const& buildability,
    std::vector<int> const& start_loc_x,
    std::vector<int> const& start_loc_y) {
  Replayer::setMap(
      h,
      w,
      walkability.data(),
      ground_height.data(),
      buildability.data(),
      start_loc_x,
      start_loc_y);
}

#define WALKABILITY_SHIFT 0
#define BUILDABILITY_SHIFT 1
#define HEIGHT_SHIFT 2
// height is 0-5, hence 3 bits
#define START_LOC_SHIFT 5

void Replayer::setMap(
    int32_t h,
    int32_t w,
    uint8_t const* const walkability,
    uint8_t const* const ground_height,
    uint8_t const* const buildability,
    std::vector<int> const& start_loc_x,
    std::vector<int> const& start_loc_y) {
  map.height = h;
  map.width = w;
  map.data.resize(h * w);
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint8_t v_w = walkability[y * w + x] & 1;
      uint8_t v_b = buildability[y * w + x] & 1;
      // Ground height only goes up to 5
      uint8_t v_g = ground_height[y * w + x] & 0b111;
      uint8_t packed = (v_w << WALKABILITY_SHIFT) |
          (v_b << BUILDABILITY_SHIFT) | (v_g << HEIGHT_SHIFT);
      map.data[y * w + x] = packed;
    }
  }
  for (size_t i = 0; i < static_cast<size_t>(start_loc_x.size()); i++) {
    auto x = start_loc_x[i];
    auto y = start_loc_y[i];
    auto v = map.data[y * w + x] | (1 << START_LOC_SHIFT);
    map.data[y * w + x] = v;
  }
}

void Replayer::setMapFromState(torchcraft::State const* state) {
  auto w = state->map_size[0];
  auto h = state->map_size[1];
  std::vector<int> start_loc_x, start_loc_y;
  for (auto pos : state->start_locations) {
    start_loc_x.push_back(pos.x);
    start_loc_y.push_back(pos.y);
  }
  Replayer::setMap(
      h,
      w,
      state->walkable_data.data(),
      state->ground_height_data.data(),
      state->buildable_data.data(),
      start_loc_x,
      start_loc_y);
}

std::pair<int32_t, int32_t> Replayer::getMap(
    std::vector<uint8_t>& walkability,
    std::vector<uint8_t>& ground_height,
    std::vector<uint8_t>& buildability,
    std::vector<int>& start_loc_x,
    std::vector<int>& start_loc_y) const {
  auto h = mapHeight();
  auto w = mapWidth();
  walkability.resize(h * w);
  ground_height.resize(h * w);
  buildability.resize(h * w);
  start_loc_x.clear();
  start_loc_y.clear();
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint8_t v = map.data[y * w + x];
      walkability[y * w + x] = (v >> WALKABILITY_SHIFT) & 1;
      buildability[y * w + x] = (v >> BUILDABILITY_SHIFT) & 1;
      ground_height[y * w + x] = (v >> HEIGHT_SHIFT) & 0b111;
      bool is_start = ((v >> START_LOC_SHIFT) & 1) == 1;
      if (is_start) {
        start_loc_x.push_back(x);
        start_loc_y.push_back(y);
      }
    }
  }
  return std::make_pair(h, w);
}

void Replayer::load(const std::string& path) {
#ifdef WITH_ZSTD
  zstd::ifstream in(path);
#else
  std::ifstream in(path);
#endif
  if (!in.good())
    throw std::runtime_error("Cannot open file to load replay");
  in >> *this;
  in.close();
}

void Replayer::save(const std::string& path, bool compressed) {
#ifndef WITH_ZSTD
  if (compressed) {
    std::cerr << "Warning: no Zstd support; disabling "
              << "compression for saved replay" << std::endl;
    compressed = false;
  }
#endif

  if (compressed) {
#ifdef WITH_ZSTD
    zstd::ofstream out(path);
    if (!out.good())
      throw std::runtime_error("Cannot open file to save replay");
    out << *this;
    out.close();
#endif
  } else {
    std::ofstream out(path);
    if (!out.good())
      throw std::runtime_error("Cannot open file to save replay");
    out << *this;
    out.close();
  }
}

} // namespace replayer
} // namespace torchcraft
