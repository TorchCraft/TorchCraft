/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef TORCHCRAFT_CONTROL_H_
#define TORCHCRAFT_CONTROL_H_

#include <fstream>
#include <vector>
#include <memory>

#include <BWAPI.h>

#include "zmq_server.h"
#include "frame.h"
#include "config_manager.h"
#include "recorder.h"

class ZMQ_server;

enum Commands {
  // without args
  QUIT, RESTART, MAP_HACK, REQUEST_IMAGE, EXIT_PROCESS, NOOP,
  // one arg
  SET_SPEED, SET_LOG, SET_GUI, SET_FRAMESKIP, SET_CMD_OPTIM,
  SET_COMBINE_FRAMES, SET_MAP, SET_MULTI,
  // arguments are those of BWAPI::UnitCommand
  COMMAND_UNIT, COMMAND_UNIT_PROTECTED,
  // variable arguments
  COMMAND_USER,
  // last command id
  COMMAND_END
};

class Controller
{
public:
  Controller(bool is_client);
  ~Controller();
  bool connect_server();
  void connect();
  void loop();
  void initGame();
  void setupHandshake();
  void handleCommand(int command, const std::vector<int>& args,
    const std::string& str);
  void handleUserCommand(int command, const std::vector<int>& args);
  BWAPI::Position getPositionFromWalkTiles(int x, int y);
  BWAPI::TilePosition getTilePositionFromWalkTiles(int x, int y);
  int getAttackFrames(int unitID);
  void endGame();
  void gameCleanUp();
  void clearLastFrame();
  void onFrame();
  void packBullets(replayer::Frame& f);
  void packResources(replayer::Frame& f, BWAPI::PlayerInterface* player);
  void packMyUnits(replayer::Frame& f);
  void packTheirUnits(replayer::Frame& f, BWAPI::PlayerInterface* player);
  void packNeutral(replayer::Frame& f);
  void addUnit(BWAPI::Unit u, replayer::Frame& frame, BWAPI::PlayerInterface* player);
  void handleEvents();
  void launchStarCraft();
  void setMap(const std::string& relative_path);
  void setWindowSize(std::pair<int, int> size);
  void setWindowPos(const std::pair<int, int> size);
  std::ofstream output_log;
  std::unique_ptr<ZMQ_server> zmq_server;
  std::vector<int> deaths;
  std::unique_ptr<Recorder> recorder_;
  bool micro_mode = false;
  bool is_client;
private:
  std::unique_ptr<ConfigManager> config_;
  bool sent_battle_end_frame = false;
  bool game_ended = false;
  bool is_winner = false;
  std::wstring sc_path_;
  std::wstring tc_path_;
  static const int pixelsPerTile = 32;
  static const int pixelsPerWalkTile = 8;
  bool logCommands = true;
  int combine_frames = 1;
  replayer::Frame *last_frame = nullptr;
  int battle_frame_count = 0;
  int frameskips = 1;
  bool too_long_play_ = false;
  bool exit_process_ = false;
  bool with_image_ = false;
};

#endif // TORCHCRAFT_CONTROL_H_
