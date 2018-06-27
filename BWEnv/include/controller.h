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

#define WIN32_LEAN_AND_MEAN
#include <BWAPI.h>

#include "config_manager.h"
#include "frame.h"
#include "recorder.h"
#include "zmq_server.h"

#include "messages_generated.h"

class ZMQ_server;
namespace replayer = torchcraft::replayer;
namespace fbs = torchcraft::fbs;

// clang-format off
enum Commands {
  // without args
  QUIT,
  RESTART,
  MAP_HACK,
  REQUEST_IMAGE,
  EXIT_PROCESS,
  NOOP,
  // one arg
  SET_SPEED,
  SET_LOG,
  SET_GUI,
  SET_FRAMESKIP,
  SET_CMD_OPTIM,
  SET_COMBINE_FRAMES, // one or two args actually
  SET_MAP,
  SET_MULTI,
  SET_BLOCKING,
  SET_MAX_FRAME_TIME_MS,
  // arguments are those of BWAPI::UnitCommand
  COMMAND_UNIT,
  COMMAND_UNIT_PROTECTED,
  // variable arguments
  COMMAND_USER, COMMAND_OPENBW,
  // BWAPI drawing routines
  DRAW_LINE, // x1, y1, x2, y2, color
  DRAW_UNIT_LINE, // uid1, uid2, color
  DRAW_UNIT_POS_LINE, // uid. x2, y2, color
  DRAW_CIRCLE, // x, y, radius, color
  DRAW_UNIT_CIRCLE, // uid, radius, color
  DRAW_TEXT, // x, y + text
  DRAW_TEXT_SCREEN, // x, y + text
  // last command id
  COMMAND_END
};
// clang-format on

enum OBWCommands {
  // two args
  KILL_UNIT,
  // four args
  SPAWN_UNIT,
  SET_PLAYER_UPGRADE_LEVEL, // player, upgrade, level
  SET_PLAYER_RESEARCHED, // player, tech, bool
  SET_PLAYER_MINERALS, // player, minerals
  SET_PLAYER_GAS, // player, gas
  SET_UNIT_HEALTH, // unit, health
  SET_UNIT_SHIELD, // unit, shield
  SET_UNIT_ENERGY, // unit, energy
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

struct FrameSerializationResults {
  flatbuffers::Offset<void> offset;
  fbs::FrameOrFrameDiff type;
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
  void packCreep(replayer::Frame& f);
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
  void setIsWinner(bool isWinner);
  void clearPendingReceive();

  std::ofstream output_log;
  std::unique_ptr<ZMQ_server> zmq_server;
  std::vector<int> deaths;
  std::unique_ptr<Recorder> recorder_;
  bool micro_mode = false;
  bool is_client;

 private:
  FrameSerializationResults serializeFrameData(flatbuffers::FlatBufferBuilder& builder);
  std::unique_ptr<ConfigManager> config_;
  bool sent_battle_end_frame = false;
  bool game_ended = false;
  bool is_winner = false;
  std::wstring sc_path_;
  std::wstring tc_path_;
  static const int pixelsPerTile = 32;
  static const int pixelsPerWalkTile = 8;
  bool logCommands = true;
  int min_combine_frames = 1;
  int max_combine_frames = -1;
  int combined_frames = 0;
  bool last_receive_ok = true;
  replayer::Frame* last_frame = nullptr;
  replayer::Frame* prev_sent_frame = nullptr; // For frame diffs
  int battle_frame_count = 0;
  int frameskips = 1;
  bool exit_process_ = false;
  bool with_image_ = false;
  bool blocking_ = true;
  int max_frame_time_ms_ = 50;
  std::vector<uint8_t> image_data_;
  std::vector<uint8_t> visibility_;
  std::vector<int8_t> commandsStatus_;
  std::vector<std::pair<std::vector<int>, std::string>> draw_cmds_;
};

#endif // TORCHCRAFT_CONTROL_H_
