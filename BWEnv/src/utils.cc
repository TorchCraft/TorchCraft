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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <BWAPI.h>
#include <BWAPI/Client.h>

#include "utils.h"

void starcraftInject(const std::wstring& command, const std::wstring& sc_path_, bool close) {
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
}

std::wstring Utils::getEnvValue(const wchar_t* env)
{
  // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx
  DWORD bufferSize = 65535;
  std::wstring buff;
  buff.resize(bufferSize);
  bufferSize = GetEnvironmentVariableW(env, &buff[0], bufferSize);
  if (!bufferSize)
  {
    // TODO add logging
    return std::wstring(L"");
  }
  //error
  buff.resize(bufferSize);
  return buff;
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

void Utils::launchSCCustom(const std::wstring& command)
{
  // TODO
}

void Utils::killStarCraft()
{
  BWAPI::BWAPIClient.disconnect();
  system("taskkill /F /T /IM StarCraft.exe");
}

void Utils::overwriteConfig(const std::wstring& sc_path_,
  const std::string& prefix,
  const std::string& arg)
{
  std::vector<std::string> filedata;
  std::wstring path = sc_path_ + L"\\bwapi-data\\bwapi.ini";
  std::ifstream ini(path);

  std::string line;

  bool found = false;

  while (getline(ini, line))
  {
    if (!line.compare(0, prefix.size(), prefix)){
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

  std::ofstream newFile(path.c_str());

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

std::string Utils::mapToTensorStr()
{
  std::stringstream r;
  r << "torch.ByteTensor({";
  for (int x = 0; x < BWAPI::Broodwar->mapHeight() * 4; ++x)
  {
    r << "{";
    for (int y = 0; y < BWAPI::Broodwar->mapWidth() * 4; ++y)
    {
      if (BWAPI::Broodwar->isWalkable(x, y)) {
        r << BWAPI::Broodwar->getGroundHeight(x / 4, y / 4) << ",";
      }
      else {
        r << "-1,";
      }
      // TODO add isBuildable?
    }
    r << "},";
  }
  r << "})";
  return r.str();
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

bool Utils::DISPLAY_LOG = false;