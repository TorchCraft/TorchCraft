/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <time.h>
#include <fstream>
#include <iostream>
#include <random>

#include "frame.h"
#include "replayer.h"

namespace replayer {

  // When playing the game, we add frames to a Replayer.
  // Once we know whether that game was won or lost, we move the replayer to
  // the correct CircularBuffer

  // just a fixed memory circular buffer for holding stacks of frames.
  class CircularBuffer{
    public:
      std::vector<Replayer*> buffer;
      size_t size, head, tail, length;
      std::mt19937 rng;
      CircularBuffer(): rng((std::random_device())()){}
      explicit CircularBuffer(size_t size)
        :size(size), head(0), tail(0), length(0) {
          buffer.resize(size);
          for (int i = 0;  i < size; i++) {
            buffer[i] = nullptr;
          }
        }

      ~CircularBuffer() {
        // free the replayers
        for (int i = 0;  i < size; i++) {
          if (buffer[i] != nullptr) {
            buffer[i]->decref();
            buffer[i] = nullptr;
          }
        }
      }

      // move the head
      void add(Replayer* r) {
        head = (head + 1) % size;
        if (buffer[head] != nullptr) {
          buffer[head]->decref();
        }
        r->incref();
        buffer[head] = r;
        if (++length > size) {
          length = size;
          tail = (tail + 1) % size;
        }
      }

      Replayer* sample() {
        size_t offset = 1 + rng() % length;
        return buffer[(tail + offset) % size]; // don't tell my mom
      }

      Replayer* getLast(size_t i) {
        return buffer[(head + size - i) % size]; // don't tell Tim's mom
      }

      // clears the buffer
      void clear() {
        for (int i = 0;  i < size; i++) {
          if (buffer[i] != nullptr) {
            delete buffer[i];
            buffer[i] = nullptr;
          }
        }
        length = 0; head = 0; tail = 0;
      }

      size_t getSize() {
        return length;
      }
  };

  std::ostream& operator<<(std::ostream& out, const CircularBuffer& o);
  std::istream& operator>>(std::istream& in, CircularBuffer& o);

  class GameStore{
    CircularBuffer lost, won;
    bool lastGameWon;
    std::mt19937 rng;
    std::uniform_real_distribution<> dist;

    public:
    GameStore() : lastGameWon(false),
    rng((std::random_device())()), dist(0, 1) {}
    explicit GameStore(size_t lost, size_t won)
      :lost(lost), won(won), lastGameWon(false), dist(0, 1) {}

    size_t getTotalSize() {
      return won.getSize() + lost.getSize();
    }

    void add(Replayer* fs, bool hasWon) {
      // if we can't sample from that frame stack, we don't add it.
      if (fs->size() >= 2) {
        if (hasWon) {
          won.add(fs);
          lastGameWon = true;
        } else {
          lost.add(fs);
          lastGameWon = false;
        }
      }
    }

    Replayer* sample(double prop_won) {
      bool inWon = dist(rng) < prop_won;
      if ((inWon && won.getSize() > 0) || lost.getSize() == 0) {
        assert(won.getSize() > 0);
        return won.sample();
      } // else
      return lost.sample();
    }

    size_t getSizeLost() {
      return lost.getSize();
    }

    Replayer* getLastLost(size_t i) {
      // gives you last battle for i=0, second to last for i=1, etc.
      assert(lost.getSize() > i);
      return lost.getLast(i);
    }

    Replayer* getLast() {
      if (lastGameWon)
      {
        assert(won.getSize() > 0);
        return won.getLast(0);
      }
      assert(lost.getSize() > 0);
      return lost.getLast(0);
    }

    friend std::ostream& operator<<(std::ostream& out, const GameStore& o);
    friend std::istream& operator>>(std::istream& in, GameStore& o);
  };


} // replayer

replayer::GameStore* checkGameStore(lua_State* L, int id = 1);

extern "C" int newGameStore(lua_State* L);
extern "C" int gcGameStore(lua_State* L);
extern "C" int loadGameStore(lua_State* L);
extern "C" int gameStoreSave(lua_State* L);
extern "C" int gameStoreAdd(lua_State* L);
extern "C" int gameStoreSample(lua_State* L);
extern "C" int gameStoreGetSizeLost(lua_State* L);
extern "C" int gameStoreGetLastBattlesLost(lua_State* L);
extern "C" int gameStoreGetLast(lua_State* L);

const struct luaL_Reg gamestore_m [] = {
  {"__gc", gcGameStore},
  {"save", gameStoreSave},
  {"add", gameStoreAdd},
  {"sample", gameStoreSample},
  {"getSizeLost", gameStoreGetSizeLost},
  {"getLastBattlesLost", gameStoreGetLastBattlesLost},
  {"getLast", gameStoreGetLast},
  {nullptr, nullptr}
};
