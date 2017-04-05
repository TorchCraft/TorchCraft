/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <thread>
#define WIN32_LEAN_AND_MEAN

#include <BWAPI/Client.h>

#include "controller.h"
#include "utils.h"
#include "user_actions.h"

#include "messages_generated.h"

Controller::Controller(bool is_client) {
  this->is_client = is_client;
  sc_path_ = Utils::envToWstring(L"STARCRAFT_DIR", L"C:/StarCraft/");
  const std::wstring tc_default_path = std::wstring(sc_path_).append(L"/TorchCraft/");
  tc_path_ = Utils::envToWstring(L"TORCHCRAFT_DIR", tc_default_path.c_str());

  // TODO when ZMQ is persistent, remember to send first map information

  config_ = std::make_unique<ConfigManager>();
  config_->loadConfig("C:/StarCraft/bwapi-data/torchcraft.ini");

  recorder_ = std::make_unique<Recorder>(config_->img_save_path);

  Utils::DISPLAY_LOG = config_->display_log;

  std::cout << *config_ << std::endl;

  this->zmq_server = std::make_unique<ZMQ_server>(this, config_->port);

  tcframe_.screen_position = std::make_unique<torchcraft::fbs::Vec2>();
  tcframe_.visibility_size = std::make_unique<torchcraft::fbs::Vec2>();
  tcframe_.img_size = std::make_unique<torchcraft::fbs::Vec2>();
}

Controller::~Controller()
{
}

bool Controller::connect_server()
{
  // connect to client
  try
  {
    this->zmq_server->connect();
  }
  catch (std::exception &e)
  {
    output_log.open(config_->log_path
      + std::to_string(this->zmq_server->getPort()) + ".txt");
    Utils::bwlog(output_log, "Error on connection: %s", e.what());
    if (this->zmq_server->server_sock_connected) {
      torchcraft::fbs::ErrorT err;
      err.message = e.what();
      this->zmq_server->sendError(&err);
      this->zmq_server->close();
    }
    return false;
  }

  assert(this->zmq_server->server_sock_connected);

  output_log.open(config_->log_path
    + std::to_string(this->zmq_server->getPort()) + ".txt");
  Utils::bwlog(output_log, "Successfully connected to proxy client on port %d.",
    this->zmq_server->getPort());
  // line 46 (onStart)

  return true;
}

void Controller::connect()
{
  while (1) {
    std::cout << "Connecting..." << std::endl;

    while (!BWAPI::BWAPIClient.connect())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
    }
    std::cout << "Connected to a StarCraft client" << std::endl;

    while (!BWAPI::Broodwar->isInGame())
    {
      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected())
      {
        break;
      }
    }
    std::cout << "Joined a game on " << BWAPI::Broodwar->mapFileName() << std::endl;
    return;
  }
}

void Controller::loop()
{
  launchStarCraft();

  while (true)
  {
    connect();

    // HACK this quits earlier than expected if incoming message contains
    // a positive exit_process command.
    initGame(); // enable some flags

    if (exit_process_)
    {
      Utils::killStarCraft();
      return;
    }

    while (BWAPI::Broodwar->isInGame())
    {
      for (auto i = 0; i < this->frameskips; i++)
      {
        BWAPI::BWAPIClient.update();
        handleEvents();
      }

      onFrame();

      if (exit_process_) break;

      if (game_ended || too_long_play_)
      {
        endGame();
        break;
      }
    }

    gameCleanUp();
    if (exit_process_)
    {
      Utils::killStarCraft();
      return;
    }

    BWAPI::BWAPIClient.update();
    BWAPI::BWAPIClient.update();
    BWAPI::BWAPIClient.update();
    BWAPI::Broodwar->leaveGame();
    // We assume automatic restart is enabled
    BWAPI::BWAPIClient.update();
  }
}

void Controller::initGame()
{
  BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

  if (is_client)
    setupHandshake();
  // TODO this is a logic hack - remove once exit_process is properly
  // fixed.
  if (exit_process_)
    return;

  // Log some information
  Utils::bwlog(output_log, "Map: %s", BWAPI::Broodwar->mapFileName().c_str());
  Utils::bwlog(output_log, "getStartLocations().size(): %d",
    BWAPI::Broodwar->getStartLocations().size());

  if (BWAPI::Broodwar->isReplay()) {
    Utils::bwlog(output_log, "This is a replay with:");
    BWAPI::Playerset players = BWAPI::Broodwar->getPlayers();
    for (auto &p : players) {
      if (p->isObserver()) {
        Utils::bwlog(output_log, "- %s (observer)", p->getName().c_str());
      }
      else {
        Utils::bwlog(output_log, "- %s (playing as %s)", p->getName().c_str(),
          p->getRace().c_str());
      }
    }
  }
  else {
    if (BWAPI::Broodwar->enemy()) {
      Utils::bwlog(output_log, "Matchup: %s vs %s",
        BWAPI::Broodwar->self()->getRace().c_str(),
        BWAPI::Broodwar->enemy()->getRace().c_str());
    }
    else {
      Utils::bwlog(output_log, "No enemy??");
    }
  }
}

void Controller::setupHandshake()
{
  torchcraft::fbs::HandshakeServerT handshake;
  handshake.lag_frames = BWAPI::Broodwar->getLatencyFrames();
  handshake.map_size.reset(new torchcraft::fbs::Vec2(BWAPI::Broodwar->mapWidth()*4, BWAPI::Broodwar->mapHeight()*4));
  handshake.ground_height_data = Utils::groundHeightToVector();
  handshake.walkable_data = Utils::walkableToVector();
  handshake.buildable_data = Utils::buildableToVector();
  handshake.map_name = BWAPI::Broodwar->mapFileName();
  handshake.neutral_id = BWAPI::Broodwar->neutral()->getID();
  if (BWAPI::Broodwar->isReplay()) {
    handshake.is_replay = true;
  } else {
    handshake.is_replay = false;
    handshake.player_id = BWAPI::Broodwar->self()->getID();
  }
  handshake.battle_frame_count = battle_frame_count;
  for (auto loc : BWAPI::Broodwar->getStartLocations()) {
    BWAPI::WalkPosition walkPos(loc);
    handshake.start_locations.emplace_back(walkPos.x, walkPos.y);
  }

  this->zmq_server->sendHandshake(&handshake);

  /* Receive first message (usually setup commands) */
  this->zmq_server->receiveMessage();
}

int8_t Controller::handleCommand(int command, const std::vector<int>& args,
  const std::string& str)
{
  int8_t status = CommandStatus::SUCCESS;
  auto check_args = [&](uint32_t n) {
    if (args.size() < n) {
      Utils::bwlog(output_log, "Missing arguments for command %d: expected %d, got %d",
        command, n, args.size());
      status = CommandStatus::MISSING_ARGUMENTS;
      return false;
    }
    return true;
  };
  auto check_unit = [&](int id) {
    auto res = BWAPI::Broodwar->getUnit(id);
    if (res == nullptr) {
      status = CommandStatus::INVALID_UNIT;
    }
    return res;
  };

  if (command <= Commands::NOOP)
  {
    switch (command) {
    case Commands::QUIT: // quit game
      Utils::bwlog(output_log, "LEAVING GAME!");
      BWAPI::Broodwar->leaveGame();
      return CommandStatus::SUCCESS;
    case Commands::RESTART: // restart game
      Utils::bwlog(output_log, "RESTARTING GAME!");
      BWAPI::Broodwar->restartGame();
      // Wait to finish game and start a new one if we're in the client...
      if (BWAPI::BWAPIClient.isConnected()) {
        while (BWAPI::Broodwar->isInGame()) BWAPI::BWAPIClient.update();
        while (!BWAPI::Broodwar->isInGame()) BWAPI::BWAPIClient.update();
      }
      return CommandStatus::SUCCESS;
    case Commands::MAP_HACK: // remove fog of war, can only be done in onStart (at init)
      Utils::bwlog(output_log, "Removing fog of war.");
      BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);
      return CommandStatus::SUCCESS;
    case Commands::REQUEST_IMAGE:
      Utils::bwlog(output_log, "Requesting image.");
      this->with_image_ = true;
      return CommandStatus::SUCCESS;
    case Commands::EXIT_PROCESS:
      Utils::bwlog(output_log, "Qutting game... Good-bye!");
      this->exit_process_ = true;
      return CommandStatus::SUCCESS;
    case Commands::NOOP:
      return CommandStatus::SUCCESS;
    }
  }
  else if (command <= Commands::SET_MULTI)
  {
    if (!check_args(1)) return status;
    switch (command) {
    case Commands::SET_SPEED:
      Utils::bwlog(output_log, "Set game speed: %d", args[0]);
      BWAPI::Broodwar->setLocalSpeed(args[0]);
      return CommandStatus::SUCCESS;
    case Commands::SET_LOG:
      Utils::bwlog(output_log, "Set logCommands to: %d", args[0]);
      logCommands = args[0] != 0;
      return CommandStatus::SUCCESS;
    case Commands::SET_GUI:
      Utils::bwlog(output_log, "Set GUI to: %d", args[0]);
      BWAPI::Broodwar->setGUI(args[0] != 0);
      return CommandStatus::SUCCESS;
    case Commands::SET_FRAMESKIP:
      Utils::bwlog(output_log, "Set frameskip to: %d", args[0]);
      BWAPI::Broodwar->setFrameSkip(args[0]);
      // this->frameskips = args[0];
      return CommandStatus::SUCCESS;
    case Commands::SET_CMD_OPTIM:
      Utils::bwlog(output_log, "Set command optimization level to: %d", args[0]);
      BWAPI::Broodwar->setCommandOptimizationLevel(args[0]);
      return CommandStatus::SUCCESS;
    case Commands::SET_COMBINE_FRAMES:
      Utils::bwlog(output_log, "Set combine frames to: %d", args[0]);
      this->combine_frames = args[0];
      return CommandStatus::SUCCESS;
    case Commands::SET_MAP:
      setMap(str);
      return CommandStatus::SUCCESS;
    case Commands::SET_MULTI:
      Utils::bwlog(output_log, "Set multiplayer: %d", args[0]);
      std::string string = args[0] ? "LAN" : "SINGLE_PLAYER";
      Utils::overwriteConfig(sc_path_, "auto_menu", string);
      Utils::overwriteConfig(sc_path_, "lan_mode", "Local PC");
      return CommandStatus::SUCCESS;
    }
  }
  else if (command <= Commands::COMMAND_UNIT_PROTECTED)
  {
    if (!check_args(2)) return status;
    auto unit = check_unit(args[0]);
    if (unit == nullptr) return status;
    auto cmd_type = args[1];
    auto target = (args.size() >= 3 ? BWAPI::Broodwar->getUnit(args[2]) : nullptr);
    BWAPI::Position position = BWAPI::Positions::Invalid;
    BWAPI::TilePosition tposition = BWAPI::TilePositions::Invalid;
    if (args.size() >= 5) {
      // Some commands require tile position
      if (cmd_type == BWAPI::UnitCommandTypes::Build ||
        cmd_type == BWAPI::UnitCommandTypes::Land ||
        cmd_type == BWAPI::UnitCommandTypes::Build_Addon ||
        cmd_type == BWAPI::UnitCommandTypes::Place_COP
        ) tposition = getTilePositionFromWalkTiles(args[3], args[4]);
      else position = getPositionFromWalkTiles(args[3], args[4]);
    }
    int x, y;
    if (position.isValid())
    {
      x = position.x;
      y = position.y;
    }
    else if (tposition.isValid())
    {
      x = tposition.x;
      y = tposition.y;
    }
    else
    {
      x = y = 0;
    }
    auto extra = (args.size() >= 6 ? args[5] : 0);
    switch (command) {
    case Commands::COMMAND_UNIT:
      Utils::bwlog(output_log, "Unit:%d command (%d, %d, (%d, %d), %d)",
        args[0], cmd_type, target,
        x, y, extra);
      if (!unit->issueCommand(BWAPI::UnitCommand(unit, cmd_type, target,
        x, y, extra))) {
        Utils::bwlog(output_log, "Commanding unit failed! Error: %s",
          BWAPI::Broodwar->getLastError().c_str());
        return CommandStatus::BWAPI_ERROR_MASK | BWAPI::Broodwar->getLastError().getID();
      }
      return CommandStatus::SUCCESS;
    case Commands::COMMAND_UNIT_PROTECTED:
      const char *msg = "OK";
      status = CommandStatus::PROTECTED;
      if (target != nullptr && unit->getTarget() == target) {
        msg = "CANCELED (already targetted)";
      }
      else if (target != nullptr
        && unit->getOrderTarget() == target) {
        msg = "CANCELED (already targetted 2)";
      }
      else if (unit->isAttackFrame()) {
        msg = "CANCELED (attack frame)";
      }
      else if (unit->getOrder() == BWAPI::Orders::AttackUnit
        && unit->getLastCommandFrame() + getAttackFrames(args[0])
        >= BWAPI::Broodwar->getFrameCount()) {
        msg = "CANCELED (still attacking)";
      }
      else {
        if (!unit->issueCommand(BWAPI::UnitCommand(unit, cmd_type,
          target, x, y, extra))) {
          Utils::bwlog(output_log, "Commanding unit failed! Error: %s",
            BWAPI::Broodwar->getLastError().c_str());
          status = CommandStatus::BWAPI_ERROR_MASK | BWAPI::Broodwar->getLastError().getID();
        } else {
          status = CommandStatus::SUCCESS;
        }
      }
      Utils::bwlog(output_log, "Unit:%d command (%d, %d, (%d, %d), %d) %s",
        args[0], cmd_type, args[2],
        x, y, extra, msg);
      return status;
    }
  }
  else if (command < Commands::COMMAND_END)
  {
    switch (command)
    {
    case Commands::DRAW_LINE:
    case Commands::DRAW_UNIT_LINE:
    case Commands::DRAW_UNIT_POS_LINE:
    case Commands::DRAW_CIRCLE:
    case Commands::DRAW_UNIT_CIRCLE:
    case Commands::DRAW_TEXT:
    case Commands::DRAW_TEXT_SCREEN: {
      static std::unordered_map<int, int> argcount = {
        {Commands::DRAW_LINE, 5}, {Commands::DRAW_UNIT_LINE, 3},
        {Commands::DRAW_UNIT_POS_LINE, 4}, {Commands::DRAW_CIRCLE, 4},
        {Commands::DRAW_UNIT_CIRCLE, 3}, {Commands::DRAW_TEXT, 2},
        {Commands::DRAW_TEXT_SCREEN, 2},
      };
      if (!check_args(argcount[command])) return status;
      std::vector<int> cmd({command});
      cmd.insert(cmd.end(), args.begin(), args.end());
      draw_cmds_.push_back({cmd, str});
      return CommandStatus::SUCCESS;
    }
    case Commands::COMMAND_USER:
      auto type = args[0];
      auto second = args.begin() + 1;
      auto last = args.end();
      std::vector<int> user_args(second, last);

      return handleUserCommand(type, user_args);
    }
  }
  Utils::bwlog(output_log, "Invalid command: %d", command);
  return CommandStatus::UNKNOWN_COMMAND;
}

int8_t Controller::handleUserCommand(int command, const std::vector<int>& args)
{
  int8_t status = CommandStatus::SUCCESS;
  auto check_args = [&](uint32_t n) {
    if (args.size() < n) {
      Utils::bwlog(output_log, "Missing arguments: expected %d, got %d", n, args.size());
      status = CommandStatus::MISSING_ARGUMENTS;
      return false;
    }
    return true;
  };

  // one argument
  if (command <= UserCommands::MOVE_SCREEN_RIGHT)
  {
    if (!check_args(1)) return status;
    switch (command)
    {
    case UserCommands::MOVE_SCREEN_UP:
      user_actions::moveScreenUp(args[0]);
      return CommandStatus::SUCCESS;
    case UserCommands::MOVE_SCREEN_DOWN:
      user_actions::moveScreenDown(args[0]);
      return CommandStatus::SUCCESS;
    case UserCommands::MOVE_SCREEN_LEFT:
      user_actions::moveScreenLeft(args[0]);
      return CommandStatus::SUCCESS;
    case UserCommands::MOVE_SCREEN_RIGHT:
      user_actions::moveScreenRight(args[0]);
      return CommandStatus::SUCCESS;
    }
  }
  else if (command < UserCommands::USER_COMMAND_END)
  {
    switch (command)
    {
    case UserCommands::MOVE_SCREEN_TO_POS:
      if (!check_args(2)) return status;
      user_actions::moveScreenToPos(args[0], args[1]);
      return CommandStatus::SUCCESS;
    case UserCommands::RIGHT_CLICK:
      if (!check_args(4)) return status;
      user_actions::rightClickPos(args[0], args[1], args[2],
        args[3] == 0 ? false : true);
      return CommandStatus::SUCCESS;
    }
  }
  Utils::bwlog(output_log, "Invalid user command: %d", command);
  return CommandStatus::UNKNOWN_COMMAND;
}

void Controller::setCommandsStatus(std::vector<int8_t> status)
{
  tcframe_.commands_status = status;
}

BWAPI::Position Controller::getPositionFromWalkTiles(int x, int y)
{
  return BWAPI::Position(pixelsPerWalkTile*x, pixelsPerWalkTile*y);
}

BWAPI::TilePosition Controller::getTilePositionFromWalkTiles(int x, int y)
{
  return BWAPI::TilePosition(x / 4, y / 4);
}

int Controller::getAttackFrames(int unitID)
{
  int attackFrames = BWAPI::Broodwar->getLatencyFrames();
  int unitType = BWAPI::Broodwar->getUnit(unitID)->getType().getID();
  // From https://docs.google.com/spreadsheets/d/1bsvPvFil-kpvEUfSG74U3E5PLSTC02JxSkiR8QdLMuw/edit#gid=0
  if (unitType == BWAPI::UnitTypes::Enum::Protoss_Dragoon) {
    attackFrames += 5;
  }
  else if (unitType == BWAPI::UnitTypes::Enum::Zerg_Devourer){
    attackFrames += 7;
  }
  return attackFrames;
}

void Controller::endGame()
{
  Utils::bwlog(output_log, "Game ended (%s)", (this->is_winner ? "WON" : "LOST"));

  torchcraft::fbs::EndGameT endg;
  if (last_frame != nullptr) {
    std::ostringstream out;
    out << *last_frame;
    auto s = out.str();
    endg.frame.assign(s.data(), s.data() + s.size());
  }
  endg.game_won = this->is_winner;

  this->zmq_server->sendEndGame(&endg);
 
  if (is_client)
  {
    // And receive new commands
    this->zmq_server->receiveMessage();
  }
  else
  {
    this->zmq_server->close();
  }
}

void Controller::gameCleanUp()
{
  this->is_winner = false;
  this->game_ended = false;
}

void Controller::clearLastFrame()
{
  if (last_frame != nullptr) {
    last_frame->decref();
    last_frame = nullptr;
  }
}

void Controller::executeDrawCommands()
{
  for (const auto& cmdpair : draw_cmds_) {
    auto cmd = cmdpair.first;
    auto text = cmdpair.second;
    switch (cmd[0]) {
      case Commands::DRAW_LINE:
        BWAPI::Broodwar->drawLineMap(cmd.at(1), cmd.at(2), cmd.at(3),
          cmd.at(4), cmd.at(5));
        break;
      case Commands::DRAW_UNIT_LINE: {
        auto unit1 = BWAPI::Broodwar->getUnit(cmd.at(1));
        auto unit2 = BWAPI::Broodwar->getUnit(cmd.at(2));
        if (unit1 != nullptr && unit2 != nullptr &&
          unit1->exists() && unit2->exists()) {
          BWAPI::Broodwar->drawLineMap(unit1->getPosition(),
            unit2->getPosition(), cmd.at(3));
        }
        break;
      }
      case Commands::DRAW_UNIT_POS_LINE: {
        auto unit = BWAPI::Broodwar->getUnit(cmd.at(1));
        if (unit != nullptr && unit->exists()) {
          auto pos = unit->getPosition();
          BWAPI::Broodwar->drawLineMap(pos.x, pos.y, cmd.at(2), cmd.at(3),
            cmd.at(4));
        }
        break;
      }
      case Commands::DRAW_CIRCLE:
        BWAPI::Broodwar->drawCircleMap(cmd.at(1), cmd.at(2), cmd.at(3),
          cmd.at(4));
        break;
      case Commands::DRAW_UNIT_CIRCLE: {
        auto unit = BWAPI::Broodwar->getUnit(cmd.at(1));
        if (unit != nullptr && unit->exists()) {
          BWAPI::Broodwar->drawCircleMap(unit->getPosition(), cmd.at(2), cmd.at(3));
        }
        break;
      }
      case Commands::DRAW_TEXT:
        BWAPI::Broodwar->drawTextMap(cmd.at(1), cmd.at(2), text.c_str());
        break;
      case Commands::DRAW_TEXT_SCREEN:
        BWAPI::Broodwar->drawTextScreen(cmd.at(1), cmd.at(2), text.c_str());
        break;
    }
  }
}

void Controller::onFrame()
{
  // Display the game frame rate as text in the upper left area of the screen
  BWAPI::Broodwar->drawTextScreen(200, 0, "FPS: %d", BWAPI::Broodwar->getFPS());
  BWAPI::Broodwar->drawTextScreen(200, 20, "Average FPS: %f", BWAPI::Broodwar->getAverageFPS());
  try {
    executeDrawCommands();
  } catch (std::exception& e) {
    Utils::bwlog(output_log, "Error drawing client annotations: %s", e.what());
  }


  // Called once every game frame
  // if more than ~2 hours worth of SC for the game, reboot the game
  too_long_play_ = Utils::checkTimeInGame();

  // check if the Proxy Bot is connected
  if (!this->zmq_server->server_sock_connected) {
    return;
  }

  // Return if the game is paused
  if (BWAPI::Broodwar->isPaused())
    return;

  // Determine whether the frame must be sent now
  bool must_send = (this->battle_frame_count % combine_frames == 0);

  // If we are in battle mode and the battle has ended, we need to:
  // 1. Send as soon as possible a frame where the Lua side can identify
  //    the winner, before all the units get destroyed
  // 2. Stop sending frames while the next battle has not started.
  bool battle_ended = micro_mode &&
    !BWAPI::Broodwar->isReplay() &&
    (BWAPI::Broodwar->self()->getUnits().empty()
    || BWAPI::Broodwar->enemy()->getUnits().empty());

  if (battle_ended) {
    must_send = !this->sent_battle_end_frame;
  }

  // Save frame state
  if (!battle_ended || !this->sent_battle_end_frame) {
    replayer::Frame *f = new replayer::Frame();

    if (BWAPI::Broodwar->isReplay()) {
      for (auto player : BWAPI::Broodwar->getPlayers()) {
        if (!player->isNeutral())
        {
          this->packTheirUnits(*f, player);
          this->packResources(*f, player);
        }
      }
      this->packNeutral(*f);
    }
    else {
      this->packResources(*f, BWAPI::Broodwar->self());
      this->packMyUnits(*f);
      this->packTheirUnits(*f, BWAPI::Broodwar->enemy());
      this->packNeutral(*f);
    }
    this->packBullets(*f);

    // Combine with last_frame
    if (last_frame == nullptr) {
      last_frame = f;
    }
    else {
      last_frame->combine(*f);
      f->decref();
    }
  }

  // Send frame out to Lua side
  if (must_send) {
    // only done once before sending

    if (with_image_)
    {
      this->tcframe_.img_mode = config_->img_mode;

      auto pos = BWAPI::Broodwar->getScreenPosition();
      this->tcframe_.screen_position->mutate_x(pos.x);
      this->tcframe_.screen_position->mutate_y(pos.y);

      // get visibility
      this->tcframe_.visibility_size->mutate_x(20);
      this->tcframe_.visibility_size->mutate_y(13);
      this->tcframe_.visibility.resize(20 * 13);

      auto init_x = pos.x / 32;
      auto init_y = pos.y / 32;
      auto it = this->tcframe_.visibility.begin();
      for (auto y = 0; y < 13; y++)
      {
        for (auto x = 0; x < 20; x++)
        {
          uint8_t tile = 0;
          if (BWAPI::Broodwar->isExplored(init_x + x, init_y + y))
            tile += 1;

          if (BWAPI::Broodwar->isVisible(init_x + x, init_y + y))
            tile += 1;

          *it++ = tile;
        }
      }
    }

    {
      std::ostringstream out;
      out << *last_frame;
      auto s = out.str();
      this->tcframe_.data.assign(s.data(), s.data() + s.size());
      clearLastFrame();
    }

    this->tcframe_.deaths = this->deaths;
    this->deaths.clear();

    this->tcframe_.frame_from_bwapi = BWAPI::Broodwar->getFrameCount();
    this->tcframe_.battle_frame_count = this->battle_frame_count;

    if (with_image_)
    {
      auto bin_data = recorder_->getScreenData(config_->img_mode, config_->window_mode, config_->window_mode_custom);
      if (bin_data->size() > 0)
      {
        this->tcframe_.img_size->mutate_x(recorder_->width);
        this->tcframe_.img_size->mutate_y(recorder_->height);
        this->tcframe_.img_data.assign(bin_data->data(), bin_data->data() + bin_data->size());
      }
      else
      {
        this->tcframe_.img_data.clear();
      }

      delete bin_data;
      with_image_ = false;
    }
    else
    {
      this->tcframe_.img_data.clear();
    }

    this->zmq_server->sendFrame(&this->tcframe_);

    // And receive new commands
    draw_cmds_.clear();
    tcframe_.commands_status.clear();
    this->zmq_server->receiveMessage();

    if (battle_ended) {
      this->sent_battle_end_frame = true;
    }
  }

  if (battle_ended) {
    this->battle_frame_count = 0;
  }
  else {
    this->battle_frame_count++;
    this->sent_battle_end_frame = false; // reset state
  }
}

/**
* Pack some information about bullets.
* The full list of BulletTypes is there https://bwapi.github.io/namespace_b_w_a_p_i_1_1_bullet_types_1_1_enum.html
*/
void Controller::packBullets(replayer::Frame &f)
{
  for (auto &b : BWAPI::Broodwar->getBullets())
  {
    if (!b->getPosition().isValid())
      continue;
    f.bullets.push_back({
      b->getType(),
      b->getPosition().x / pixelsPerWalkTile,
      b->getPosition().y / pixelsPerWalkTile
    });
  }
}

/**
* Pack information about resources.
*/
void Controller::packResources(replayer::Frame &f, BWAPI::PlayerInterface* p)
{
  f.resources[p->getID()] = {
    p->minerals(), p->gas(), p->supplyUsed(), p->supplyTotal()
  };
}

void Controller::packMyUnits(replayer::Frame& f)
{
  for (auto &u : BWAPI::Broodwar->self()->getUnits())
  {
    // Ignore the unit if it no longer exists
    // Make sure to include this block when handling any Unit pointer!
    if (!u->exists())
      continue;

    if (u->getHitPoints() > 0) {
      addUnit(u, f, BWAPI::Broodwar->self()); // TODO: only when the state changes
    }
  }
}

void Controller::packTheirUnits(replayer::Frame &f, BWAPI::PlayerInterface* player)
{
  for (auto &u : player->getUnits())
  {
    if (u->getHitPoints() > 0)
    {
      addUnit(u, f, player); // TODO: only when the state changes
    }
  }
}

void Controller::packNeutral(replayer::Frame &f)
{
  for (auto &u : BWAPI::Broodwar->getNeutralUnits())
  {
    if (u->getType().isMineralField() || u->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
    {
      addUnit(u, f, BWAPI::Broodwar->neutral()); // TODO: only when the state changes
    }
  }
}

/**
* Pack some information about the state of the unit:
* unit ID: integer
* position: (x, y) integers couple (in walk tiles)
* enemy: 0/1
* unit type: integer
* current hit points: integer
* current shield points: integer
* ground weapon cooldown: integer
* air weapon cooldown: integer
* status flags: integer
* is visible?: integer
* some other stuff...
* see http://bwapi.github.io/class_b_w_a_p_i_1_1_unit_interface.html for all that's available
*/
void Controller::addUnit(BWAPI::Unit u, replayer::Frame& frame, BWAPI::PlayerInterface* player)
{
  BWAPI::Position unitPosition = u->getPosition();
  if (!unitPosition.isValid())
    return;
  int x_wt = unitPosition.x / pixelsPerWalkTile;
  int y_wt = unitPosition.y / pixelsPerWalkTile;

  int pixel_size_x = u->getType().width();
  int pixel_size_y = u->getType().height();
  int unit_player = u->getPlayer()->getID();

  BWAPI::UnitType utype = u->getType();
  // could player->damage(WeaponType wpn) here but not sure of #attacks
  int ground_attack = (utype.groundWeapon().damageAmount()
    + utype.groundWeapon().damageBonus()
    * player->getUpgradeLevel(utype.groundWeapon().upgradeType()))
    * utype.maxGroundHits() * utype.groundWeapon().damageFactor();
  int air_attack = (utype.airWeapon().damageAmount()
    + utype.airWeapon().damageBonus()
    * player->getUpgradeLevel(utype.airWeapon().upgradeType()))
    * utype.maxAirHits() * utype.airWeapon().damageFactor();
  // store visibility from each player in a separate bit
  int32_t visible = 0;
  for (auto player: BWAPI::Broodwar->getPlayers()) {
    if (player->getID() >= 0) {
      visible |= (u->isVisible(player) << player->getID());
    }
  }

  uint64_t flags = 0;
  flags |= u->isAccelerating() ? replayer::Unit::Flags::Accelerating : 0;
  flags |= u->isAttacking() ? replayer::Unit::Flags::Attacking : 0;
  flags |= u->isAttackFrame() ? replayer::Unit::Flags::AttackFrame : 0;
  flags |= u->isBeingConstructed() ? replayer::Unit::Flags::BeingConstructed : 0;
  flags |= u->isBeingGathered() ? replayer::Unit::Flags::BeingGathered : 0;
  flags |= u->isBeingHealed() ? replayer::Unit::Flags::BeingHealed : 0;
  flags |= u->isBlind() ? replayer::Unit::Flags::Blind : 0;
  flags |= u->isBraking() ? replayer::Unit::Flags::Braking : 0;
  flags |= u->isBurrowed() ? replayer::Unit::Flags::Burrowed : 0;
  flags |= u->isCarryingGas() ? replayer::Unit::Flags::CarryingGas : 0;
  flags |= u->isCarryingMinerals() ? replayer::Unit::Flags::CarryingMinerals : 0;
  flags |= u->isCloaked() ? replayer::Unit::Flags::Cloaked : 0;
  flags |= u->isCompleted() ? replayer::Unit::Flags::Completed : 0;
  flags |= u->isConstructing() ? replayer::Unit::Flags::Constructing : 0;
  flags |= u->isDefenseMatrixed() ? replayer::Unit::Flags::DefenseMatrixed : 0;
  flags |= u->isDetected() ? replayer::Unit::Flags::Detected : 0;
  flags |= u->isEnsnared() ? replayer::Unit::Flags::Ensnared : 0;
  flags |= u->isFlying() ? replayer::Unit::Flags::Flying : 0;
  flags |= u->isFollowing() ? replayer::Unit::Flags::Following : 0;
  flags |= u->isGatheringGas() ? replayer::Unit::Flags::GatheringGas : 0;
  flags |= u->isGatheringMinerals() ? replayer::Unit::Flags::GatheringMinerals : 0;
  flags |= u->isHallucination() ? replayer::Unit::Flags::Hallucination : 0;
  flags |= u->isHoldingPosition() ? replayer::Unit::Flags::HoldingPosition : 0;
  flags |= u->isIdle() ? replayer::Unit::Flags::Idle : 0;
  flags |= u->isInterruptible() ? replayer::Unit::Flags::Interruptible : 0;
  flags |= u->isInvincible() ? replayer::Unit::Flags::Invincible : 0;
  flags |= u->isIrradiated() ? replayer::Unit::Flags::Irradiated : 0;
  flags |= u->isLifted() ? replayer::Unit::Flags::Lifted : 0;
  flags |= u->isLoaded() ? replayer::Unit::Flags::Loaded : 0;
  flags |= u->isLockedDown() ? replayer::Unit::Flags::LockedDown : 0;
  flags |= u->isMaelstrommed() ? replayer::Unit::Flags::Maelstrommed : 0;
  flags |= u->isMorphing() ? replayer::Unit::Flags::Morphing : 0;
  flags |= u->isMoving() ? replayer::Unit::Flags::Moving : 0;
  flags |= u->isParasited() ? replayer::Unit::Flags::Parasited : 0;
  flags |= u->isPatrolling() ? replayer::Unit::Flags::Patrolling : 0;
  flags |= u->isPlagued() ? replayer::Unit::Flags::Plagued : 0;
  flags |= u->isPowered() ? replayer::Unit::Flags::Powered : 0;
  flags |= u->isRepairing() ? replayer::Unit::Flags::Repairing : 0;
  flags |= u->isResearching() ? replayer::Unit::Flags::Researching : 0;
  flags |= u->isSelected() ? replayer::Unit::Flags::Selected : 0;
  flags |= u->isSieged() ? replayer::Unit::Flags::Sieged : 0;
  flags |= u->isStartingAttack() ? replayer::Unit::Flags::StartingAttack : 0;
  flags |= u->isStasised() ? replayer::Unit::Flags::Stasised : 0;
  flags |= u->isStimmed() ? replayer::Unit::Flags::Stimmed : 0;
  flags |= u->isStuck() ? replayer::Unit::Flags::Stuck : 0;
  flags |= u->isTargetable() ? replayer::Unit::Flags::Targetable : 0;
  flags |= u->isTraining() ? replayer::Unit::Flags::Training : 0;
  flags |= u->isUnderAttack() ? replayer::Unit::Flags::UnderAttack : 0;
  flags |= u->isUnderDarkSwarm() ? replayer::Unit::Flags::UnderDarkSwarm : 0;
  flags |= u->isUnderDisruptionWeb() ? replayer::Unit::Flags::UnderDisruptionWeb : 0;
  flags |= u->isUnderStorm() ? replayer::Unit::Flags::UnderStorm : 0;
  flags |= u->isUpgrading() ? replayer::Unit::Flags::Upgrading : 0;

  frame.units[player->getID()].push_back({
    u->getID(), x_wt, y_wt,
    u->getHitPoints(), utype.maxHitPoints(),
    u->getShields(), utype.maxShields(), u->getEnergy(),
    player->weaponDamageCooldown(utype),
    u->getGroundWeaponCooldown(), u->getAirWeaponCooldown(),
    flags,
    visible,
    utype.getID(),
    player->armor(utype),
    player->getUpgradeLevel(BWAPI::UpgradeTypes::Protoss_Plasma_Shields),
    utype.size().getID(),
    unitPosition.x,
    unitPosition.y,
    pixel_size_x,
    pixel_size_y,
    ground_attack, air_attack,
    utype.groundWeapon().damageType().getID(),
    utype.airWeapon().damageType().getID(),
    player->weaponMaxRange(utype.groundWeapon()) / pixelsPerWalkTile,
    player->weaponMaxRange(utype.airWeapon()) / pixelsPerWalkTile,
    std::vector<replayer::Order>(),
    replayer::UnitCommand(),
    u->getVelocityX(),
    u->getVelocityY(),
    unit_player,
    u->getResources(),
  });

  // Add curent order to order list
  // (we keep Orders::None orders as their timing marks the moment where
  //  previous order stops)
  int targetid = -1;
  if (u->getTarget()) {
    targetid = u->getTarget()->getID();
  }
  else if (u->getOrderTarget()) {
    targetid = u->getOrderTarget()->getID();
  }
  BWAPI::Position targetpos = u->getTargetPosition();

  frame.units[player->getID()].back().orders.push_back({
    BWAPI::Broodwar->getFrameCount(),      // first frame
    u->getOrder().getID(),
    targetid,
    targetpos.isValid() ? targetpos.x / pixelsPerWalkTile : -1,
    targetpos.isValid() ? targetpos.y / pixelsPerWalkTile : -1
  });

  // Set last command
  auto& command = frame.units[player->getID()].back().command;
  auto lastCommand = u->getLastCommand();
  targetpos = lastCommand.getTargetPosition();
  command.frame = u->getLastCommandFrame();
  command.type = lastCommand.type.getID();
  if (lastCommand.target) {
    command.targetId = lastCommand.target->getID();
  } else {
    command.targetId = -1;
  }
  command.targetX = targetpos.isValid() ? targetpos.x / pixelsPerWalkTile : -1;
  command.targetY = targetpos.isValid() ? targetpos.y / pixelsPerWalkTile : -1;
  command.extra = lastCommand.extra;
}

void Controller::handleEvents()
{
  for (auto &e : BWAPI::Broodwar->getEvents())
  {
    switch (e.getType())
    {
    case BWAPI::EventType::UnitDestroy:
      deaths.push_back(e.getUnit()->getID());
      break;
    case BWAPI::EventType::MatchEnd:
      this->game_ended = true;
      this->is_winner = e.isWinner();
      break;
    default:
      break;
    }
  }
}

void Controller::launchStarCraft()
{
  if (config_->assume_on)
    return;

  if (config_->launcher == "bwheadless")
  {
    Utils::launchSCWithBWheadless(sc_path_, tc_path_);
  }
  else if (config_->launcher == "injectory")
  {
    Utils::launchSCWithInjectory(sc_path_, tc_path_);
  }
  else if (config_->launcher == "custom")
  {
    std::string custom_launcher = config_->custom_launcher;
    std::wstring command(custom_launcher.length(), L' ');
    std::copy(custom_launcher.begin(), custom_launcher.end(), command.begin());
    Utils::launchSCCustom(sc_path_, command);
  }
  else {
    // TODO decide what to do here
    // I'd rather the user failed with bad config, as opposed to
    // defaulting to bwheadless.
    throw std::exception();
  }
}

void Controller::setMap(const std::string& relative_path)
{
  Utils::bwlog(output_log, "Set map: %s", relative_path.c_str());
  Utils::overwriteConfig(sc_path_, "map", relative_path);
  std::string path(sc_path_.begin(), sc_path_.end()); // Can't set wstr paths in bwapi anyway...
  if (BWAPI::BroodwarPtr)
    if (!BWAPI::Broodwar->setMap(path + "/" + relative_path)) {
      Utils::bwlog(output_log, "Set map to %s failed! Error: %s",
        path + "/" + relative_path,
        BWAPI::Broodwar->getLastError().c_str());
    }
}

void Controller::setWindowSize(const std::pair<int, int> size)
{
  Utils::bwlog(output_log, "Set window size: %d, %d", size.first, size.second);
  Utils::overwriteConfig(sc_path_, "width", std::to_string(size.first));
  Utils::overwriteConfig(sc_path_, "height", std::to_string(size.second));
}

void Controller::setWindowPos(const std::pair<int, int> pos)
{
  Utils::bwlog(output_log, "Set window pos: %d, %d", pos.first, pos.second);
  Utils::overwriteConfig(sc_path_, "left", std::to_string(pos.first));
  Utils::overwriteConfig(sc_path_, "top", std::to_string(pos.second));
}
