/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <sstream>
#include <fstream>
#include <codecvt>
#include <regex>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <BWAPI.h>
#include <BWAPI/Client.h>

#include "utils.h"

void starcraftInject(const std::wstring& command, const std::wstring& sc_path_, bool close) {
#ifdef _WIN32
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcess(
    nullptr,
    const_cast<wchar_t *>(command.c_str()),
    nullptr,
    nullptr,
    FALSE,
    0,
    nullptr,
    sc_path_.c_str(),
    &si,
    &pi)
    )
  {
    std::cout << "While running command:\n\t" << const_cast<wchar_t *>(command.c_str())
      << "\nCreateProcess failed: " << GetLastError()
      << std::endl;
  }

  // Injector quits just after injecting
  if (close) {
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
#else
  throw std::runtime_error("starcraftInject: This setting only works on Windows");
#endif
}

std::wstring Utils::getEnvValue(const wchar_t* env)
{
  char* r = std::getenv(ws2s(env).c_str());
  if (!r) return {};
  return s2ws(r);
}

std::wstring Utils::envToWstring(const wchar_t* env, const wchar_t* def)
{
  auto ws = getEnvValue(env);
  if (ws.length() == 0)
  {
    // TODO add logging
    return std::wstring(def);
  }
  return ws;
}

void Utils::launchSCWithBWheadless(const std::wstring& sc_path_, const std::wstring& tc_path_)
{
  std::wstring command = L"";
  command += tc_path_ + L"\\BWEnv\\bin\\bwheadless.exe --headful ";
  command += L"-e " + sc_path_ + L"\\StarCraft.exe ";
  command += L"-l " + sc_path_ + L"\\bwapi-data\\BWAPI.dll";
  std::cout << std::string(command.begin(), command.end()) << std::endl;

  starcraftInject(command, sc_path_, false);
}

void Utils::launchSCWithInjectory(const std::wstring& sc_path_, const std::wstring& tc_path_)
{
  // TODO Check if this works on Wine.
  std::wstring command = L"";
  command += tc_path_ + L"\\BWEnv\\bin\\injectory.x86.exe ";
  command += L"--launch " + sc_path_ + L"\\StarCraft.exe ";
  command += L"--inject " + sc_path_ + L"\\bwapi-data\\BWAPI.dll";
  std::cout << std::string(command.begin(), command.end()) << std::endl;

  starcraftInject(command, sc_path_, true);
}

void Utils::launchSCCustom(const std::wstring& sc_path_, const std::wstring& command)
{
  std::cout << std::string(command.begin(), command.end()) << std::endl;
  starcraftInject(command, sc_path_, false);
}

void Utils::killStarCraft()
{
  BWAPI::BWAPIClient.disconnect();
#ifndef OPENBW_BWAPI
#ifdef _WIN32
  system("taskkill /F /T /IM StarCraft.exe");
#endif
#endif
}

void Utils::overwriteConfig(const std::wstring& sc_path_,
  const std::string& prefix,
  const std::string& arg)
{
  std::vector<std::string> filedata;
  std::wstring path = sc_path_ + L"\\bwapi-data\\bwapi.ini";
  std::ifstream ini(ws2s(path));

  std::string line;
  std::regex regex("\\s*" + prefix + "\\s*=.*");

  bool found = false;

  while (getline(ini, line))
  {
    if (std::regex_match(line, regex)) {
      filedata.push_back(prefix + " = " + arg);
      found = true;
    }
    else
      filedata.push_back(line);
  }
  ini.close();

  if (!found)
  {
    // TODO throw some exception here
    // bwlog("`%s` was not found in file.", map.c_str());
  }

  std::ofstream newFile(ws2s(path).c_str());

  if (newFile.is_open())
  {
    for (auto s : filedata)
      newFile << s << "\n";
  }
  else
  {
    // TODO throw some exception here
    // bwlog("Cannot open INI file.");
  }

  newFile.close();
}

void Utils::bwlog(std::ofstream& output_log,
  std::string format,
  ...)
{
  static char stringbuffer[2048];

  va_list args;
  if (output_log) {
    va_start(args, format);
    vsnprintf(stringbuffer, 2048, format.c_str(), args);
    output_log << stringbuffer << std::endl;
  }
  va_start(args, format);
  if (BWAPI::BWAPIClient.isConnected() && DISPLAY_LOG)
    BWAPI::Broodwar->vPrintf(format.c_str(), args);
  va_end(args);
}

std::vector<uint8_t> Utils::groundHeightToVector()
{
  std::vector<uint8_t> v;
  for (int y = 0; y < BWAPI::Broodwar->mapHeight() * 4; ++y) {
    for (int x = 0; x < BWAPI::Broodwar->mapWidth() * 4; ++x) {
      v.push_back(BWAPI::Broodwar->getGroundHeight(x/4, y/4));
    }
  }
  return v;
}

std::vector<uint8_t> Utils::walkableToVector()
{
  std::vector<uint8_t> v;
  for (int y = 0; y < BWAPI::Broodwar->mapHeight() * 4; ++y) {
    for (int x = 0; x < BWAPI::Broodwar->mapWidth() * 4; ++x) {
      v.push_back(BWAPI::Broodwar->isWalkable(x, y));
    }
  }
  return v;
}

std::vector<uint8_t> Utils::buildableToVector()
{
  std::vector<uint8_t> v;
  for (int y = 0; y < BWAPI::Broodwar->mapHeight() * 4; ++y) {
    for (int x = 0; x < BWAPI::Broodwar->mapWidth() * 4; ++x) {
      v.push_back(BWAPI::Broodwar->isBuildable(x/4, y/4));
    }
  }
  return v;
}

bool Utils::checkTimeInGame()
{
  // more than ~2 hours, restart the game
  int max_frames = 2 * 60 * 60 * 24;

  return BWAPI::Broodwar->getFrameCount() > max_frames;
}

std::string Utils::ws2s(const std::wstring& ws)
{
  using convert_typeX = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_typeX, wchar_t> converterX;
  return converterX.to_bytes(ws);
}

std::wstring Utils::s2ws(const std::string& s)
{
  using convert_typeX = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_typeX, wchar_t> converterX;
  return converterX.from_bytes(s);
}

std::string readIni(const std::string& filename, const std::string& section, const std::string& key) {
  FILE* f = fopen(filename.c_str(), "rb");
  if (!f) return {};
  std::vector<char> data;
  fseek(f, 0, SEEK_END);
  long filesize = ftell(f);
  data.resize(filesize);
  fseek(f, 0, SEEK_SET);
  fread(data.data(), filesize, 1, f);
  fclose(f);
  bool correct_section = section.empty();
  const char* c = data.data();
  const char* e = c + data.size();
  auto whitespace = [&]() {
    switch (*c) {
    case ' ': case '\t': case '\v': case '\f': case '\r':
      return true;
    default:
      return false;
    }
  };
  while (c != e) {
    while (c != e && (whitespace() || *c == '\n')) ++c;
    if (c == e) break;
    if (*c == '#' || *c == ';') {
      while (c != e && *c != '\n') ++c;
    } else if (*c == '[') {
      correct_section = false;
      ++c;
      const char* n = c;
      while (c != e && *c != ']' && *c != '\n') {
        ++c;
      }
      if (!section.compare(0, section.size(), n, c - n)) correct_section = true;
      if (c != e) ++c;
    } else {
      const char* n = c;
      while (c != e && !whitespace() && *c != '=' && *c != '\n') {
        ++c;
      }
      if (c != e) {
        if (correct_section && !key.compare(0, key.size(), n, c - n)) {
          while (c != e && whitespace()) ++c;
          if (c != e && *c == '=') {
            ++c;
            while (c != e && whitespace()) ++c;
            n = c;
            while (c != e && *c != '\r' && *c != '\n') ++c;
            return std::string(n, c - n);
          }
        } else {
          while (c != e && *c != '\n') ++c;
        }
      }
    }
  }
  return {};
}

std::string readIniString(const std::string& section, const std::string& key, const std::string& default_, const std::string& filename) {
  auto s = readIni(filename, section, key);
  if (s.empty()) s = default_;
  return s;
}

int readIniInt(const std::string& section, const std::string& key, int default_, const std::string& filename) {
  auto s = readIni(filename, section, key);
  if (s.empty()) return default_;
  return (int)std::atoi(s.c_str());
}

bool Utils::DISPLAY_LOG = false;
