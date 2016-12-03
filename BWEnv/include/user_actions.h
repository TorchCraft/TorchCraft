/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef TC_USER_ACTIONS_H_
#define TC_USER_ACTIONS_H_

enum UserCommands
{
  // one arg
  MOVE_SCREEN_UP, MOVE_SCREEN_DOWN, MOVE_SCREEN_LEFT, MOVE_SCREEN_RIGHT,
  // multiple args
  MOVE_SCREEN_TO_POS, RIGHT_CLICK,
  // last command id
  USER_COMMAND_END
};

namespace user_actions
{
void moveScreenUp(int magnitude);
void moveScreenDown(int magnitude);
void moveScreenLeft(int magnitude);
void moveScreenRight(int magnitude);
void moveScreenToPos(int x, int y);
void rightClickPos(int unit_id, int x, int y, bool queue);
} // user_actions

#endif // TC_USER_ACTIONS_H_