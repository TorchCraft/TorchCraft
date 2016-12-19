/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>
#include <propidl.h>
#include <config_manager.h>
#include <algorithm>
#include <sstream>

#include "utils.h"

ConfigManager::ConfigManager()
{
  loaded_ = false;
  // general
  port = 0;
  display_log = false;
  // starcraft
  assume_on = false;
}

void ConfigManager::loadConfig(std::string path)
{
  loaded_ = true;
  current_path_ = path;
  loadGeneralSection();
  loadStarCraftSection();
}

void ConfigManager::loadDefault()
{
  loadConfig("");
}

void ConfigManager::loadGeneralSection()
{
  port = readInt_("general", "port", 0);
  log_path = readString_("general", "log_path", "C:/tc_data/torchcraft_log_cpp_port_");
  display_log = readBool_("general", "display_log", "false");
  img_mode = readString_("general", "img_mode", "raw");
  window_mode = readString_("general", "window_mode", "windows");
  window_mode_custom = readString_("general", "window_mode_custom", "");
  img_save_path = readString_("general", "img_save_path", "C:/tc_data/output_");
}

void ConfigManager::loadStarCraftSection()
{
  assume_on = readBool_("starcraft", "assume_on", "false");
  launcher = readString_("starcraft", "launcher", "injectory");
  custom_launcher = readString_("starcraft", "custom_launcher", "");
}

bool ConfigManager::isLoaded()
{
  return loaded_;
}

bool ConfigManager::readBool_(const char* section,
  const char* key,
  const char* defaultVal)
{
  auto str = readString_(section, key, defaultVal);
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  std::istringstream is(str);
  bool b;
  is >> std::boolalpha >> b;
  return b;
}

std::string ConfigManager::readString_(const char* section,
  const char* key,
  const char* defaultVal)
{
  std::string val;
  auto u = std::string(key);

  std::transform(u.begin(), u.end(), u.begin(), ::toupper);

  auto env = Utils::s2ws("TORCHCRAFT_" + u);
  auto ws = Utils::getEnvValue(env.c_str());
  if (ws.length() > 0)
  {
    val = Utils::ws2s(ws);
  }
  else {
    char* temp = new char[255];
    GetPrivateProfileStringA(section, key, defaultVal, temp, 255, current_path_.c_str());
    val = temp;
    delete[] temp;
  }
  return val;
}

int ConfigManager::readInt_(const char* section,
  const char* key,
  int defaultVal)
{
  int val;
  auto u = std::string(key);
  std::transform(u.begin(), u.end(), u.begin(), ::toupper);
  auto env = Utils::s2ws("TORCHCRAFT_" + u);
  auto ws = Utils::getEnvValue(env.c_str());
  if (ws.length() > 0)
  {
    val = std::stoi(Utils::ws2s(ws));
  }
  else {
    val = GetPrivateProfileIntA(section, key, defaultVal, current_path_.c_str());
  }
  return val;
}

std::string ConfigManager::toString() const
{
  std::stringstream data;
  data << "<Config Info>" << std::endl;
  data << "  loaded: " << loaded_ << std::endl;
  data << "  current path: " << current_path_ << std::endl;
  data << "general" << std::endl;
  data << "  port = " << port << std::endl;
  data << "  log_path = " << log_path << std::endl;
  data << "  display_log = " << display_log << std::endl;
  data << "  img_mode = " << img_mode << std::endl;
  data << "  window_mode = " << window_mode << std::endl;
  data << "  window_mode_custom = " << window_mode_custom << std::endl;
  data << "  img_save_path = " << img_save_path << std::endl;
  data << "starcraft" << std::endl;
  data << "  assume_on = " << assume_on << std::endl;
  data << "  launcher = " << launcher << std::endl;
  data << "  custom_launcher = " << custom_launcher << std::endl;

  return data.str();
}

std::ostream& operator<<(std::ostream &strm, const ConfigManager &cm) {
  return strm << cm.toString();
}