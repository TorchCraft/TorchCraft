/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <atomic>

/*
 *  Reference counting for Frames and Replayers.
 *
 *  Enables frames and replayers to be referenced by Lua variables and other
 *  C++ objects at the same time.
 *
 *  We cannot use a C++ shared_pointer because that doesn't help us with the
 *  interaction with Lua.
 */
class RefCounted {
  private:
    std::atomic_int refs;

  public:
    RefCounted() {
      refs = 1;
    }
    // Destructor needs to be virtual for delete this to work correctly.
    virtual ~RefCounted() {}

    void incref() {
      refs++;
    }
    void decref() {
      if (--refs == 0) delete this;
    }
};
