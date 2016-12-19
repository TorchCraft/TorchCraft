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
      std::ostringstream out;
      this->zmq_server->packMessage("error = true");
      out << "error_msg = [[" << e.what() << "]]";
      this->zmq_server->packMessage(out.str());
      this->zmq_server->sendMessage();
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
  /* Pack and send a specific message with all setup variables */
  this->zmq_server->packMessage(std::string("lag_frames = " +
    std::to_string(BWAPI::Broodwar->getLatencyFrames())));
  this->zmq_server->packMessage(std::string("map_data = " +
    Utils::mapToTensorStr()));
  this->zmq_server->packMessage(std::string("map_name = '" +
    BWAPI::Broodwar->mapFileName() + "'"));

  if (BWAPI::Broodwar->isReplay()) {
    this->zmq_server->packMessage("is_replay = true");
  }
  else {
    this->zmq_server->packMessage("is_replay = false");
    this->zmq_server->packMessage(std::string("player_id = " +
      std::to_string(BWAPI::Broodwar->self()->getID())));
    this->zmq_server->packMessage(std::string("neutral_id = " +
      std::to_string(BWAPI::Broodwar->neutral()->getID())));
  }

  if (micro_mode)
    this->zmq_server->packMessage("battle_frame_count = "
    + std::to_string(battle_frame_count));

  this->zmq_server->sendMessage();

  /* Receive first message (usually setup commands) */
  this->zmq_server->receiveMessage();

  battle_frame_count = 0;
}

void Controller::handleCommand(int command, const std::vector<int>& args,
  const std::string& str)
{
  auto check_args = [&](uint32_t n) {
    if (args.size() < n) throw(std::runtime_error("Not enough arguments."));
  };
  auto check_unit = [&](int id) {
    auto res = BWAPI::Broodwar->getUnit(id);
    if (res) return res;
    throw(std::runtime_error("Bad unit id."));
  };
  if (command <= Commands::NOOP)
  {
    switch (command) {
    case Commands::QUIT: // quit game
      Utils::bwlog(output_log, "LEAVING GAME!");
      BWAPI::Broodwar->leaveGame();
      return;
    case Commands::RESTART: // restart game
      Utils::bwlog(output_log, "RESTARTING GAME!");
      BWAPI::Broodwar->restartGame();
      // Wait to finish game and start a new one if we're in the client...
      if (BWAPI::BWAPIClient.isConnected()) {
        while (BWAPI::Broodwar->isInGame()) BWAPI::BWAPIClient.update();
        while (!BWAPI::Broodwar->isInGame()) BWAPI::BWAPIClient.update();
      }
      return;
    case Commands::MAP_HACK: // remove fog of war, can only be done in onStart (at init)
      Utils::bwlog(output_log, "Removing fog of war.");
      BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);
      return;
    case Commands::REQUEST_IMAGE:
      Utils::bwlog(output_log, "Requesting image.");
      this->with_image_ = true;
      return;
    case Commands::EXIT_PROCESS:
      Utils::bwlog(output_log, "Qutting game... Good-bye!");
      this->exit_process_ = true;
      return;
    case Commands::NOOP:
      return;
    }
  }
  else if (command <= Commands::SET_MULTI)
  {
    check_args(1);
    switch (command) {
    case Commands::SET_SPEED:
      Utils::bwlog(output_log, "Set game speed: %d", args[0]);
      BWAPI::Broodwar->setLocalSpeed(args[0]);
      return;
    case Commands::SET_LOG:
      Utils::bwlog(output_log, "Set logCommands to: %d", args[0]);
      logCommands = args[0] != 0;
      return;
    case Commands::SET_GUI:
      Utils::bwlog(output_log, "Set GUI to: %d", args[0]);
      BWAPI::Broodwar->setGUI(args[0] != 0);
      return;
    case Commands::SET_FRAMESKIP:
      Utils::bwlog(output_log, "Set frameskip to: %d", args[0]);
      BWAPI::Broodwar->setFrameSkip(args[0]);
      // this->frameskips = args[0];
      return;
    case Commands::SET_CMD_OPTIM:
      Utils::bwlog(output_log, "Set command optimization level to: %d", args[0]);
      BWAPI::Broodwar->setCommandOptimizationLevel(args[0]);
      return;
    case Commands::SET_COMBINE_FRAMES:
      Utils::bwlog(output_log, "Set combine frames to: %d", args[0]);
      this->combine_frames = args[0];
      return;
    case Commands::SET_MAP:
      setMap(str);
      return;
    case Commands::SET_MULTI:
      Utils::bwlog(output_log, "Set multiplayer: %d", args[0]);
      std::string string = args[0] ? "LAN" : "SINGLE_PLAYER";
      Utils::overwriteConfig(sc_path_, "auto_menu", string);
      Utils::overwriteConfig(sc_path_, "lan_mode", "Local PC");
      return;
    }
  }
  else if (command <= Commands::COMMAND_UNIT_PROTECTED)
  {
    check_args(2);
    auto unit = check_unit(args[0]);
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
    auto extra = (args.size() >= 6 ? args[5] : 0);
    switch (command) {
    case Commands::COMMAND_UNIT:
      Utils::bwlog(output_log, "Unit:%d command (%d, %d, (%d, %d), %d)",
        args[0], cmd_type, args[2],
        x, y, extra);
      if (!unit->issueCommand(BWAPI::UnitCommand(unit, cmd_type, target,
        x, y, extra))
        ) Utils::bwlog(output_log, "Commanding unit failed! Error: %s",
        BWAPI::Broodwar->getLastError().c_str());
      return;
    case Commands::COMMAND_UNIT_PROTECTED:
      const char *status = "OK";
      if (target != nullptr && unit->getTarget() == target) {
        status = "CANCELED (already targetted)";
      }
      else if (target != nullptr
        && unit->getOrderTarget() == target) {
        status = "CANCELED (already targetted 2)";
      }
      else if (unit->isAttackFrame()) {
        status = "CANCELED (attack frame)";
      }
      else if (unit->getOrder() == BWAPI::Orders::AttackUnit
        && unit->getLastCommandFrame() + getAttackFrames(args[0])
        >= BWAPI::Broodwar->getFrameCount()) {
        status = "CANCELED (still attacking)";
      }
      else {
        if (!unit->issueCommand(BWAPI::UnitCommand(unit, cmd_type,
          target, x, y, extra))
          ) Utils::bwlog(output_log, "Commanding unit failed! Error: %s",
          BWAPI::Broodwar->getLastError().c_str());
      }
      Utils::bwlog(output_log, "Unit:%d command (%d, %d, (%d, %d), %d) %s",
        args[0], cmd_type, args[2],
        x, y, extra, status);
      return;
    }
  }
  else if (command < Commands::COMMAND_END)
  {
    switch (command)
    {
    case Commands::COMMAND_USER:
      auto type = args[0];
      auto second = args.begin() + 1;
      auto last = args.end();
      std::vector<int> user_args(second, last);

      handleUserCommand(type, user_args);
      return;
    }
  }
  Utils::bwlog(output_log, "Invalid command: %d", command);
}

void Controller::handleUserCommand(int command, const std::vector<int>& args)
{
  auto check_args = [&](uint32_t n) {
    if (args.size() < n) throw(std::runtime_error("Not enough arguments."));
  };

  // one argument
  if (command <= UserCommands::MOVE_SCREEN_RIGHT)
  {
    check_args(1);
    switch (command)
    {
    case UserCommands::MOVE_SCREEN_UP:
      user_actions::moveScreenUp(args[0]);
      return;
    case UserCommands::MOVE_SCREEN_DOWN:
      user_actions::moveScreenDown(args[0]);
      return;
    case UserCommands::MOVE_SCREEN_LEFT:
      user_actions::moveScreenLeft(args[0]);
      return;
    case UserCommands::MOVE_SCREEN_RIGHT:
      user_actions::moveScreenRight(args[0]);
      return;
    }
  }
  else if (command < UserCommands::USER_COMMAND_END)
  {
    switch (command)
    {
    case UserCommands::MOVE_SCREEN_TO_POS:
      check_args(2);
      user_actions::moveScreenToPos(args[0], args[1]);
      return;
    case UserCommands::RIGHT_CLICK:
      check_args(4);
      user_actions::rightClickPos(args[0], args[1], args[2],
        args[3] == 0 ? false : true);
      return;
    }
  }
  Utils::bwlog(output_log, "Invalid user command: %d", command);
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

  if (last_frame != nullptr) {
    std::ostringstream out;
    out << "frame=[[" << *last_frame << "]]";
    this->zmq_server->packMessage(out.str());
    clearLastFrame();
  }

  this->zmq_server->packMessage("game_ended = true");
  this->zmq_server->packMessage("game_won = " +
    std::string(this->is_winner ? "true" : "false"));
  this->zmq_server->sendMessage();

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

void Controller::onFrame()
{
  // Display the game frame rate as text in the upper left area of the screen
  BWAPI::Broodwar->drawTextScreen(200, 0, "FPS: %d", BWAPI::Broodwar->getFPS());
  BWAPI::Broodwar->drawTextScreen(200, 20, "Average FPS: %f", BWAPI::Broodwar->getAverageFPS());

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
      std::string img_mode = "img_mode = \"" + config_->img_mode + "\"";
      this->zmq_server->packMessage(img_mode);

      auto pos = BWAPI::Broodwar->getScreenPosition();
      std::stringstream spos;
      spos << "screen_position = { " << pos.x << ", " << pos.y << " }";
      this->zmq_server->packMessage(spos.str());

      // get visibility
      std::stringstream vis;
      // This is going to be a table of tables, max size 20 x 13
      auto init_x = pos.x / 32;
      auto init_y = pos.y / 32;
      vis << "visibility={";

      for (auto y = 0; y < 13; y++)
      {
        vis << "{";
        for (auto x = 0; x < 20; x++)
        {
          auto tile = 0;
          if (BWAPI::Broodwar->isExplored(init_x + x, init_y + y))
            tile += 1;

          if (BWAPI::Broodwar->isVisible(init_x + x, init_y + y))
            tile += 1;

          vis << tile << ",";
        }
        vis << "},";
      }

      vis << "}";

      this->zmq_server->packMessage(vis.str());
    }

    {
      std::ostringstream out;
      out << "frame=[[" << *last_frame << "]]";
      this->zmq_server->packMessage(out.str());
      clearLastFrame();
    }

    {
      std::ostringstream out;
      out << "deaths={";
      for (auto i : this->deaths) {
        out << i << ",";
      }
      out << "}";
      this->zmq_server->packMessage(out.str());
      deaths.clear();
    }

    this->zmq_server->packMessage("frame_from_bwapi = "
      + std::to_string(BWAPI::Broodwar->getFrameCount()));
    this->zmq_server->packMessage("battle_frame_count = "
      + std::to_string(this->battle_frame_count));

    if (with_image_)
    {
      auto bin_data = recorder_->getScreenData(config_->img_mode, config_->window_mode, config_->window_mode_custom);
      if (bin_data->size() > 0)
      {
        auto s = ("TCIMAGEDATA" + std::to_string(recorder_->width)
          + "," + std::to_string(recorder_->height) + ",");
        this->zmq_server->packMessage(s + *bin_data);
      }

      delete bin_data;
      with_image_ = false;
    }

    this->zmq_server->sendMessage();

    // And receive new commands
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

    // Ignore the unit if it has one of the following status ailments
    if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
      continue;

    // Ignore the unit if it is in one of the following states
    if (u->isLoaded() || !u->isPowered() || u->isStuck())
      continue;

    // Ignore the unit if it is incomplete or busy constructing
    if (!u->isCompleted() || u->isConstructing())
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
* is idle?: boolean
* is visible?: boolean
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

  frame.units[player->getID()].push_back({
    u->getID(), x_wt, y_wt,
    u->getHitPoints(), utype.maxHitPoints(),
    u->getShields(), utype.maxShields(), u->getEnergy(),
    player->weaponDamageCooldown(utype),
    u->getGroundWeaponCooldown(), u->getAirWeaponCooldown(),
    u->isIdle(), u->isVisible(),
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
    u->getVelocityX(),
    u->getVelocityY(),
    unit_player,
    u->getResources()
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
    // TODO
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