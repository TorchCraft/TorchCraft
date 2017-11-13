/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "lest/lest.hpp"

// Enable auto-registration
#define TCASE(name) lest_CASE(specification(), name)

#undef SCENARIO
#define SCENARIO(name) lest_CASE(specification(), lest::text("Scenario: ") + name)

extern lest::tests& specification();
