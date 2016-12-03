/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef RECORDER_H
#define RECORDER_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <vector>
#include <chrono>

// Defines the modes that the Recorder can use
enum UpdateModes
{
  GET,
  SAVE,
};

class Recorder
{
public:
  Recorder(std::string save_folder_path);
  ~Recorder();

  // Finds the StarCraft window and saves the handle (pointer) to it.
  void getBWScreen(std::string mode, std::string custom);

  // Saves grabbed screen as a bmp. The name of the image will change with the
  // frames; see getName_() for that.
  void saveImage();

  std::string* getScreenData(std::string mode_grab, std::string mode_window,
    std::string custom_window_cut);

  // Stores the last grabbed pixel data
  std::vector<uint8_t> last_image;
  uint32_t width;
  uint32_t height;
  uint16_t BitsPerPixel;

  // Stores the last timestamp recorded
  std::chrono::milliseconds last_timestamp;
  std::string save_path_;
  std::vector<int> rect_window_;
  std::vector<int> rect_ubuntu_unity_;

private:

  // Gets the format encoder for GDI+ Bitmaps
  int getEncoderClsid_(const WCHAR* format, CLSID* pClsid);

  // Take the bitmap handle and saves the GBR(NULL) pixel data into last_image
  void HBITMAPToPixels_();

  // Get a new string every time the function gets called
  std::wstring getName_();

  // Stores the last recorded handle (pointer) to the screen bitmap mask
  HBITMAP last_handle_;

  // Stores the token for GDI+
  ULONG_PTR gdiplusToken_;
};

void testPixel(std::vector<uint8_t>& v, int index);
void testPixels(std::vector<uint8_t>& v);

#endif // !RECORDER_H