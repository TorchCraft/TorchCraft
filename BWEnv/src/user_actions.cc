/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <BWAPI.h>

#include "user_actions.h"

namespace user_actions
{
void moveScreenUp(int magnitude)
{
  BWAPI::Position p = BWAPI::Broodwar->getScreenPosition();
  p.x += 0;
  p.y += -magnitude;
  BWAPI::Broodwar->setScreenPosition(p);
}

void moveScreenDown(int magnitude)
{
  BWAPI::Position p = BWAPI::Broodwar->getScreenPosition();
  p.x += 0;
  p.y += magnitude;
  BWAPI::Broodwar->setScreenPosition(p);
}

void moveScreenLeft(int magnitude)
{
  BWAPI::Position p = BWAPI::Broodwar->getScreenPosition();
  p.x += -magnitude;
  p.y += 0;
  BWAPI::Broodwar->setScreenPosition(p);
}

void moveScreenRight(int magnitude)
{
  BWAPI::Position p = BWAPI::Broodwar->getScreenPosition();
  p.x += magnitude;
  p.y += 0;
  BWAPI::Broodwar->setScreenPosition(p);
}

void moveScreenToPos(int x, int y)
{
  BWAPI::Position p = BWAPI::Broodwar->getScreenPosition();
  p.x += x;
  p.y += y;
  BWAPI::Broodwar->setScreenPosition(p);
}

void rightClickPos(int unit_id, int x, int y, bool queue)
{
  BWAPI::Position p(x, y);
  auto u = BWAPI::Broodwar->getUnit(unit_id);

  if (!(u->exists() || u->isBeingConstructed())
    || !u->canRightClickPosition(true))
    throw std::exception("Unable to take action on the unit");

  u->rightClick(p);
}
} // user_actions