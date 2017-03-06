/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <iostream>
#include <thread>

#include <gflags/gflags.h>
#include <torchcraft/client.h>
#include <torchcraft/constants.h>
#include <torchcraft/state.h>

namespace tc = torchcraft;

// CLI flags
DEFINE_string(hostname, "localhost", "Host running torchcraft server");
DEFINE_int32(port, 11111, "Port for torchcraft");
DEFINE_string(heuristic, "wc", "Heuristic");
DEFINE_int32(skip_frames, 7, "Take action every n frames");
DEFINE_bool(micro_mode, true, "Set to false for normal maps");

// Determine the closest unit to a given position
template <typename It>
It getClosest(int x, int y, It first, It last) {
  It closest = last;
  float mind = FLT_MAX;
  while (first != last) {
    float d = float(x - first->x) * (x - first->x) +
        float(y - first->y) * (y - first->y);
    if (d < mind) {
      closest = first;
      mind = d;
    }
    ++first;
  }
  return closest;
}

// Retrieve all units that are mineral fields
std::vector<tc::replayer::Unit> getMineralFields(tc::State* state) {
  auto neutralUnits = state->units[state->neutral_id];
  std::vector<tc::replayer::Unit> fields;
  std::copy_if(
      neutralUnits.begin(),
      neutralUnits.end(),
      std::back_inserter(fields),
      [](const tc::replayer::Unit& u) {
        return (
            u.type == tc::BW::UnitType::Resource_Mineral_Field ||
            u.type == tc::BW::UnitType::Resource_Mineral_Field_Type_2 ||
            u.type == tc::BW::UnitType::Resource_Mineral_Field_Type_3);
      });
  return fields;
}

// Check whether a unit's current orders include the given command
bool isExecutingCommand(
    const tc::replayer::Unit& unit,
    tc::BW::UnitCommandType command) {
  auto orders = tc::BW::commandToOrders(command);
  auto res = std::find_first_of(
      unit.orders.begin(),
      unit.orders.end(),
      orders.begin(),
      orders.end(),
      [](const tc::replayer::Order& o1, tc::BW::Order o2) {
        return o1.type == o2;
      });
  return res != unit.orders.end();
}

// Establish connection and perform initial handshake
std::unique_ptr<tc::Client> makeClient() {
  // Establish connection
  auto client = std::unique_ptr<tc::Client>(new tc::Client());
  if (!client->connect(FLAGS_hostname, FLAGS_port, 30000)) {
    throw std::runtime_error(
        std::string("Error establishing connection: ") + client->error());
  }

  // Perform handshake
  tc::Client::Options opts;
  opts.micro_battles = FLAGS_micro_mode;
  std::vector<std::string> upd;
  if (!client->init(upd, opts)) {
    throw std::runtime_error(
        std::string("Error initializing connection: ") + client->error());
  }
  if (client->state()->replay) {
    throw std::runtime_error("Expected non-replay map");
  }

  return client;
}

void playGame(std::shared_ptr<tc::Client> cl, int* totalBattles) {
  // First message: set up some variables
  std::vector<tc::Client::Command> comms;
  comms.emplace_back(tc::BW::Command::SetSpeed, 0);
  comms.emplace_back(tc::BW::Command::SetGui, 1);
  comms.emplace_back(tc::BW::Command::SetCmdOptim, 1);
  if (!cl->send(comms)) {
    throw std::runtime_error(std::string("Sent failure: ") + cl->error());
  }

  int builtBarracks = 0;
  int battlesWon = 0;
  int battlesGame = 0;
  int nloop = 0;

  auto last = std::chrono::system_clock::now();
  while (!cl->state()->game_ended) {
    // Display progress
    printf("Loop = %5d | ", nloop);
    printf(
        "FPS = %5lld | ",
        CLOCKS_PER_SEC / (std::chrono::system_clock::now() - last).count());
    printf("WR = %1.3f | ", battlesWon / (battlesGame + 1e-6));
    printf("#Wins = %4d | ", battlesWon);
    printf("#Bttls = %4d | ", battlesGame);
    printf("Tot Bttls = %4d |    ", *totalBattles);
    printf("\r");
    std::cout << std::flush;
    last = std::chrono::system_clock::now();

    std::vector<std::string> updates;
    if (!cl->receive(updates)) {
      throw std::runtime_error(std::string("Receive failure: ") + cl->error());
    }

    const auto state = cl->state();
    const auto myUnits = state->units[state->player_id];
    std::vector<tc::Client::Command> actions;
    nloop++;

    if (state->game_ended) {
      break;
    } else if (state->battle_just_ended) {
      if (state->battle_won) {
        battlesWon++;
      }
      battlesGame++;
      (*totalBattles)++;
      if (battlesGame >= 10) {
        actions.emplace_back(tc::BW::Command::Restart);
      }
      goto send;
    } else if (state->waiting_for_restart) {
      // Do nothing
      goto send;
    } else if (state->battle_frame_count % FLAGS_skip_frames != 0) {
      // Skip this frame
      goto send;
    }

    // Determine actions for each unit
    for (auto unit : myUnits) {
      auto utype = tc::BW::UnitType::_from_integral(unit.type);
      if (tc::BW::isBuilding(utype)) {
        if (utype == +tc::BW::UnitType::Terran_Barracks) {
          // Train a new marine unit
          actions.emplace_back(
              tc::BW::Command::CommandUnit,
              unit.id,
              tc::BW::UnitCommandType::Train,
              tc::BW::UnitType::Terran_Marine);
        }
      } else if (tc::BW::isWorker(utype)) {
        // Build barracks if ore count allows for it
        if (state->frame->resources[state->player_id].ore >= 150 &&
            state->frame_from_bwapi - builtBarracks > 240) {
          builtBarracks = state->frame_from_bwapi;
          auto ccenter = std::find_if(
              myUnits.begin(), myUnits.end(), [](tc::replayer::Unit u) {
                return u.type == tc::BW::UnitType::Terran_Command_Center;
              });
          if (ccenter != myUnits.end()) {
            // Already executing build order?
            if (!isExecutingCommand(unit, tc::BW::UnitCommandType::Build) &&
                !isExecutingCommand(
                    unit, tc::BW::UnitCommandType::Right_Click_Position)) {
              actions.emplace_back(
                  tc::BW::Command::CommandUnit,
                  unit.id,
                  tc::BW::UnitCommandType::Build,
                  -1,
                  ccenter->x,
                  ccenter->y + 8,
                  tc::BW::UnitType::Terran_Barracks);
            }
          }
        } else {
          // Gather resources
          if (!isExecutingCommand(unit, tc::BW::UnitCommandType::Gather) &&
              !isExecutingCommand(unit, tc::BW::UnitCommandType::Build) &&
              !isExecutingCommand(
                  unit, tc::BW::UnitCommandType::Right_Click_Position)) {
            auto mineralFields = getMineralFields(state);
            auto target = getClosest(
                unit.x, unit.y, mineralFields.begin(), mineralFields.end());
            actions.emplace_back(
                tc::BW::Command::CommandUnit,
                unit.id,
                tc::BW::UnitCommandType::Right_Click_Unit,
                target->id);
          }
        }
      } else {
        // Attack closest enemy
        auto enemyUnits = state->units[1 - state->player_id];
        auto target =
            getClosest(unit.x, unit.y, enemyUnits.begin(), enemyUnits.end());
        if (target != enemyUnits.end()) {
          actions.emplace_back(
              tc::BW::Command::CommandUnitProtected,
              unit.id,
              tc::BW::UnitCommandType::Attack_Unit,
              target->id);
        }
      }
    }

  // Send actions
  send:
    if (!cl->send(actions)) {
      throw std::runtime_error(std::string("Send failure: ") + cl->error());
    }
  }
}

// Program entry point
int main(int argc, char** argv) {
  torchcraft::init();
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  int totalBattles = 0;
  while (totalBattles < 40) {
    std::cout << std::endl << "CTRL-C to stop" << std::endl << std::endl;

    try {
      auto cl = makeClient();
      playGame(std::move(cl), &totalBattles);
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return EXIT_FAILURE;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << std::endl;
  }

  return EXIT_SUCCESS;
}
