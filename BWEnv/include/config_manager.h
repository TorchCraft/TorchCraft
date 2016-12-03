/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef CONFIG_MANAGER_H_
#define CONFIG_MANAGER_H_
#include <string>

class ConfigManager
{
public:
  ConfigManager();
  void loadConfig(std::string path);
  void loadDefault();
  void loadGeneralSection();
  void loadStarCraftSection();
  bool isLoaded();
  std::string toString() const;

  int port;
  bool assume_on;
  std::string launcher;
  std::string custom_launcher;
  std::string log_path;
  std::string img_mode;
  std::string img_save_path;
  std::string window_mode;
  std::string window_mode_custom;
  bool display_log;
private:
  std::string readString_(const char* section,
    const char* key,
    const char* defaultVal);
  int ConfigManager::readInt_(const char* section,
    const char* key,
    int defaultVal);
  bool readBool_(const char* section,
    const char* key,
    const char* defaultVal);

  // not initialising everything in constructor.
  // Explictly have to load something or default!
  bool loaded_;
  std::string current_path_;
};

std::ostream& operator<<(std::ostream &strm, const ConfigManager &cm);

#endif // !CONFIG_MANAGER_H