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
  // DG TODO: Implement
  return out;
}

std::istream& operator>>(std::istream& in, FrameDiff& o) {
  // DG TODO: Implement
  return in;
}

} // namespace replayer
} // namespace torchcraft
