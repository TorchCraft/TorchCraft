/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef TORCHCRAFT_UTILS_H_
#define TORCHCRAFT_UTILS_H_

namespace Utils
{
std::wstring getEnvValue(const wchar_t* env);
std::wstring envToWstring(const wchar_t* env, const wchar_t* def);

// StarCraft control
void launchSCWithBWheadless(const std::wstring& sc_path_,
  const std::wstring& tc_path_);
void launchSCWithInjectory(const std::wstring& sc_path_,
  const std::wstring& tc_path);
void launchSCCustom(const std::wstring& command);
void killStarCraft();

// map changing related stuff
void overwriteConfig(const std::wstring& sc_path_,
  const std::string& prefix,
  const std::string& map);

// require BWAPI thread
void bwlog(std::ofstream& output_log, std::string format, ...);
std::string mapToTensorStr();
bool checkTimeInGame();
std::string ws2s(const std::wstring& ws);
std::wstring s2ws(const std::string& s);

// Let's consider making a Utils class at some point
extern bool DISPLAY_LOG;
}

#endif // TORCHCRAFT_UTILS_H_
