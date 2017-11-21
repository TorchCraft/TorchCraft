/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <algorithm>

#include "frame.h"

namespace torchcraft {
namespace replayer {

std::ostream& operator<<(std::ostream& out, const FrameDiff& o) {
  out << o.pids.size();
  for (auto& pid : o.pids)
    out << " " << pid;
  for (auto& player_unit : o.units) {
    out << " " << player_unit.size();
    for (auto& du : player_unit)
      out << " " << du;
  }
  out << " " << o.creep_map.size();
  for (auto pair : o.creep_map)
    out << " " << pair.first << " " << pair.second;
  out << " ";
  writeTail(out, o.actions, o.resources, o.bullets);
  out << " " << o.reward << " " << o.is_terminal;
  return out;
}

std::ostream& operator<<(
    std::ostream& out,
    const detail::UnitDiff& o) {
  out << o.id << " " << o.velocityX << " " << o.velocityY;
  out << " " << o.flags;
  out << " " << o.var_ids.size();
  for (size_t i = 0; i < o.var_ids.size(); i++)
    out << " " << o.var_ids[i];
  for (size_t i = 0; i < o.var_ids.size(); i++)
    out << " " << o.var_diffs[i];
  out << " " << o.order_size << " " << o.order_ids.size();
  for (size_t i = 0; i < o.order_ids.size(); i++)
    out << " " << o.order_ids[i];
  for (size_t i = 0; i < o.order_ids.size(); i++)
    out << " " << o.order_diffs[i];
  return out;
}

std::istream& operator>>(std::istream& in, FrameDiff& o) {
  int32_t npids, n_creep_map_diff;
  in >> npids;
  o.pids.resize(npids);
  o.units.resize(npids);
  for (size_t i = 0; i < static_cast<size_t>(npids); i++)
    in >> o.pids[i];
  for (size_t i = 0, nunits = 0; i < static_cast<size_t>(npids); i++) {
    in >> nunits;
    o.units[i].resize(nunits);
    for (size_t k = 0; k < nunits; k++)
      in >> o.units[i][k];
  }
  in >> n_creep_map_diff;
  for (size_t i = 0; i < static_cast<size_t>(n_creep_map_diff); i++) {
    int32_t first, second;
    in >> first >> second;
    o.creep_map.insert(std::make_pair(first, second));
  }
  readTail(in, o.actions, o.resources, o.bullets);
  in >> o.reward >> o.is_terminal;
  return in;
}

std::istream& operator>>(std::istream& in, detail::UnitDiff& o) {
  size_t nvars, norders;

  in >> o.id >> o.velocityX >> o.velocityY;
  in >> o.flags;
  in >> nvars;
  o.var_ids.resize(nvars);
  o.var_diffs.resize(nvars);
  for (size_t i = 0; i < nvars; i++)
    in >> o.var_ids[i];
  for (size_t i = 0; i < nvars; i++)
    in >> o.var_diffs[i];
  in >> o.order_size >> norders;
  o.order_ids.resize(norders);
  o.order_diffs.resize(norders);
  for (size_t i = 0; i < norders; i++)
    in >> o.order_ids[i];
  for (size_t i = 0; i < norders; i++)
    in >> o.order_diffs[i];
  return in;
}

} // namespace replayer
} // namespace torchcraft
