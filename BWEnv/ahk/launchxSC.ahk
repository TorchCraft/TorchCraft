; Copyright (c) 2015-present, Facebook, Inc.
; All rights reserved.
; 
; This source code is licensed under the BSD-style license found in the
; LICENSE file in the root directory of this source tree. An additional grant
; of patent rights can be found in the PATENTS file in the same directory.

if 0 < 1
{
  MsgBox Input number to run as an argument!
  ExitApp
}

Run, "C:\Program Files (x86)\BWAPI\Chaoslauncher\Chaoslauncher - MultiInstance.exe"
Sleep, 1000


n = %1%
count := 0
while count < n {

  WinActivate, Chaoslauncher
  Send {Click 60,364} 

  Sleep, 200
  WinGet, count, Count, Brood
}

ExitApp
Esc::ExitApp
