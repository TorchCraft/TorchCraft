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
#include <memory>
#include <vector>

#include <BWAPI.h>

#include "config_manager.h"
#include "frame.h"
#include "recorder.h"
#include "zmq_server.h"

#include "messages_generated.h"

class ZMQ_server;
namespace replayer = torchcraft::replayer;

// clang-format off
enum Commands {
  // without args
  QUIT, RESTART, MAP_HACK, REQUEST_IMAGE, EXIT_PROCESS, NOOP,
  // one arg
  SET_SPEED, SET_LOG, SET_GUI, SET_FRAMESKIP, SET_CMD_OPTIM,
  SET_COMBINE_FRAMES, SET_MAP, SET_MULTI,
  // arguments are those of BWAPI::UnitCommand
  COMMAND_UNIT, COMMAND_UNIT_PROTECTED,
  // variable arguments
  COMMAND_USER, COMMAND_OPENBW,
  // BAWPI drawing routins
  DRAW_LINE,          // x1, y1, x2, y2, color
  DRAW_UNIT_LINE,     // uid1, uid2, color
  DRAW_UNIT_POS_LINE, // uid. x2, y2, color
  DRAW_CIRCLE,        // x, y, radius, color
  DRAW_UNIT_CIRCLE,   // uid, radius, color
  DRAW_TEXT,          // x, y + text
  DRAW_TEXT_SCREEN,   // x, y + text
  // last command id
  COMMAND_END
};
// clang-format on

enum OBWCommands {
  // two args
  KILL_UNIT,
  // four args
  SPAWN_UNIT,
};

enum CommandStatus : int8_t {
  SUCCESS = 0,
  // Positive numbers correspond to
  // BWAPI error codes from BWAPI::Errors::Enum | BWAPI_ERROR
  // (since an error code of 0 also signals an error in BWAPI)
  BWAPI_ERROR_MASK = 0x40,
  UNKNOWN_ERROR = -1,
  UNKNOWN_COMMAND = -2,
  MISSING_ARGUMENTS = -3,
  TOO_MANY_COMMANDS = -4,
  INVALID_UNIT = -5,
  PROTECTED = -6,
  OPENBW_NOT_IN_USE = -7,
  INVALID_PLAYER = -8,
  OPENBW_UNSUCCESSFUL_COMMAND = -9, // TODO reconsider whether we want this
};

class Controller {
 public:
  Controller(bool is_client);
  ~Controller();
  bool connect_server();
  void connect();
  void loop();
  void initGame();
  void setupHandshake();
  int8_t handleCommand(
      int command,
      const std::vector<int>& args,
      const std::string& str);
  int8_t handleUserCommand(int command, const std::vector<int>& args);
  int8_t handleOpenBWCommand(int command, const std::vector<int>& args);
  void setCommandsStatus(std::vector<int8_t> status);
  BWAPI::Position getPositionFromWalkTiles(int x, int y);
  BWAPI::TilePosition getTilePositionFromWalkTiles(int x, int y);
  int getAttackFrames(int unitID);
  void endGame();
  void gameCleanUp();
  void resetFrameState();
  void executeDrawCommands();
  void onFrame();
  void packBullets(replayer::Frame& f);
  void packResources(replayer::Frame& f, BWAPI::PlayerInterface* player);
  void packMyUnits(replayer::Frame& f);
  void packTheirUnits(replayer::Frame& f, BWAPI::PlayerInterface* player);
  void packNeutral(replayer::Frame& f);
  void addUnit(
      BWAPI::Unit u,
      replayer::Frame& frame,
      BWAPI::PlayerInterface* player);
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
  void Controller::serializeFrameData(torchcraft::fbs::FrameDataT*);
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
  replayer::Frame* last_frame = nullptr;
  replayer::Frame* prev_sent_frame = nullptr; // For frame diffs
  int battle_frame_count = 0;
  int frameskips = 1;
  bool too_long_play_ = false;
  bool exit_process_ = false;
  bool with_image_ = false;
  std::vector<std::pair<std::vector<int>, std::string>> draw_cmds_;
  torchcraft::fbs::FrameT tcframe_;
};

#endif // TORCHCRAFT_CONTROL_H_
