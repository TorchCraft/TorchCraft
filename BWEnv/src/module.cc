/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "Module.h"
#include <controller.h>

Module::~Module()
{
  c_->clearLastFrame();
}

void Module::onStart()
{
  this->c_ = std::make_unique<Controller>(false);
  if (!this->c_->connect_server())
  {
    BWAPI::Broodwar->leaveGame();
    return;
  }
  this->c_->initGame();
}

void Module::onEnd(bool isWinner)
{
  this->c_->endGame();
}

void Module::onFrame()
{
  this->c_->onFrame();
}

void Module::onSendText(std::string text)
{
  // Send the text to the game if it is not being processed.
  BWAPI::Broodwar->sendText("%s", text.c_str());
  // Make sure to use %s and pass the text as a parameter,
  // otherwise you may run into problems when you use the %(percent) character!
}

void Module::onReceiveText(BWAPI::Player player, std::string text)
{
  // Parse the received text
  BWAPI::Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void Module::onPlayerLeft(BWAPI::Player player)
{
  // Interact verbally with the other players in the game by
  // announcing that the other player has left.
  BWAPI::Broodwar->sendText("Goodbye %s!", player->getName().c_str());
  this->c_->zmq_server->packMessage(std::string("player_left = '"
    + player->getName() + "'").c_str());
  this->c_->zmq_server->sendMessage();
  this->c_->zmq_server->receiveMessage();
}

void Module::onNukeDetect(BWAPI::Position target)
{
  // Check if the target is a valid position
  if (target)
  {
    // if so, print the location of the nuclear strike target
    BWAPI::Broodwar << "Nuclear Launch Detected at " << target << std::endl;
  }
  else
  {
    // Otherwise, ask other players where the nuke is!
    BWAPI::Broodwar->sendText("Where's the nuke?");
  }
  // You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void Module::onUnitDiscover(BWAPI::Unit unit)
{
}

void Module::onUnitEvade(BWAPI::Unit unit)
{
}

void Module::onUnitShow(BWAPI::Unit unit)
{
}

void Module::onUnitHide(BWAPI::Unit unit)
{
}

void Module::onUnitCreate(BWAPI::Unit unit)
{
  if (BWAPI::Broodwar->isReplay())
  {
    // if we are in a replay, then we will print out the build order of the structures
    if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
    {
      int seconds = BWAPI::Broodwar->getFrameCount() / 24;
      int minutes = seconds / 60;
      seconds %= 60;
      BWAPI::Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes,
        seconds,
        unit->getPlayer()->getName().c_str(),
        unit->getType().c_str());
    }
  }
}

void Module::onUnitDestroy(BWAPI::Unit unit)
{
  this->c_->deaths.push_back(unit->getID());
}

void Module::onUnitMorph(BWAPI::Unit unit)
{
  if (BWAPI::Broodwar->isReplay())
  {
    // if we are in a replay, then we will print out the build order of the structures
    if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
    {
      int seconds = BWAPI::Broodwar->getFrameCount() / 24;
      int minutes = seconds / 60;
      seconds %= 60;
      BWAPI::Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes,
        seconds,
        unit->getPlayer()->getName().c_str(),
        unit->getType().c_str());
    }
  }
}

void Module::onUnitRenegade(BWAPI::Unit unit)
{
}

void Module::onSaveGame(std::string gameName)
{
  BWAPI::Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void Module::onUnitComplete(BWAPI::Unit unit)
{
}