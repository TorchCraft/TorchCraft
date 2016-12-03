/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#define WIN32_LEAN_AND_MEAN
#include <Unknwn.h>
// hack to get gdiplus to compile
#include <algorithm>
#include <memory>

namespace Gdiplus
{
using std::min;
using std::max;
};

#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")
// end hack
#include <sstream>
#include <iostream>
#include <bitset>
#include "recorder.h"

Recorder::Recorder(std::string save_folder_path)
{
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, nullptr);
  last_handle_ = HBITMAP();
  width = 0;
  height = 0;
  BitsPerPixel = 0;
  save_path_ = save_folder_path;

  rect_window_ = { 10, -10, 30, -10 };
  rect_ubuntu_unity_ = { 10, -10, 30, -10 };
}

Recorder::~Recorder()
{
  Gdiplus::GdiplusShutdown(gdiplusToken_);
}

// TODO(Check if method can be optimized)
void Recorder::getBWScreen(std::string mode, std::string custom)
{
  // The window cannot be minimized
  // TODO (this can be optimized if we assume the window never moves.
  // Make sure to try this later.)
  DeleteObject(last_handle_);

  RECT rc;
  HWND hwnd = FindWindow(nullptr, TEXT("Brood War"));
  if (!hwnd)
  {
    // TODO(Write error)
  }
  GetWindowRect(hwnd, &rc); // TODO: check getWindowRect

  // Remove window pixels
  std::vector<int> target;

  if (mode == "windows")
    target = rect_window_;
  else if (mode == "ubuntu_unity")
    target = rect_ubuntu_unity_;
  else if (mode == "custom")
  {
    // expecting something like <int>,<int>,<int>,<int>
    std::stringstream ss(custom);
    std::string token;
    while (getline(ss, token, ',')) {
      target.push_back(std::stoi(token));
    }
  }

  rc.left += target[0];
  rc.right += target[1];
  rc.top += target[2];
  rc.bottom += target[3];
  HDC hdcScreen = GetDC(nullptr);
  HDC hdc = CreateCompatibleDC(hdcScreen);
  last_handle_ = CreateCompatibleBitmap(hdcScreen, rc.right - rc.left,
    rc.bottom - rc.top);
  SelectObject(hdc, last_handle_);
  BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcScreen,
    rc.left, rc.top, SRCCOPY);
  DeleteObject(hdc);

  // DeleteObject(hdcScreen);
  ReleaseDC(hwnd, hdcScreen);
}

void Recorder::saveImage()
{
  Gdiplus::Bitmap* image = new Gdiplus::Bitmap(last_handle_, nullptr); // probably needed
  CLSID myClsId;
  getEncoderClsid_(L"image/bmp", &myClsId); // TODO should use returned value somehow
  std::wstring w = getName_();
  const WCHAR* wc = w.c_str();
  image->Save(wc, &myClsId, nullptr);
  delete image;
}

std::string* Recorder::getScreenData(std::string mode_grab, std::string mode_window,
  std::string custom_window_cut)
{
  std::string* bin_data = new std::string("");
  getBWScreen(mode_window, custom_window_cut);

  if (mode_grab == "raw")
  {
    HBITMAPToPixels_();
    bin_data = new std::string(reinterpret_cast<char*>(&last_image[0]), last_image.size());
  }
  if (mode_grab == "compressed")
  {
    // TODO
  }
  else if (mode_grab == "diff")
  {
    // TODO
  }
  // TODO make sure we can choose the folder of destination via torchcraft.ini
  else if (mode_grab == "save")
  {
    saveImage();
  }

  return bin_data;
}

// counter gets initialized only once
std::wstring Recorder::getName_()
{
  static std::wstringstream ss;
  static int counter = 0;
  ss.str(L"");
  ss.clear();
  // The following line will fail miserably if the path includes non ASCII chars
  ss << std::wstring(save_path_.begin(), save_path_.end()) << counter << L".bmp";
  counter++;
  return ss.str();
}

// The pixels are in GBR format, but the actual bytes contain also a final byte
// per triplet (so 32 bits in total), which is unused.
void Recorder::HBITMAPToPixels_()
{
  if (last_handle_ == nullptr)
  {
    throw std::logic_error("Null Pointer Exception. BitmapHandle is Null.");
  }

  last_image.clear();

  BITMAP Bmp = { 0 };
  BITMAPINFO Info = { 0 };
  HDC DC = CreateCompatibleDC(nullptr);
  std::memset(&Info, 0, sizeof(BITMAPINFO));
  HBITMAP OldBitmap = static_cast<HBITMAP>(SelectObject(DC, last_handle_));
  GetObject(last_handle_, sizeof(Bmp), &Bmp);
  Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  Info.bmiHeader.biWidth = width = Bmp.bmWidth;
  Info.bmiHeader.biHeight = Bmp.bmHeight * -1;
  height = Bmp.bmHeight;
  Info.bmiHeader.biPlanes = 1;
  Info.bmiHeader.biBitCount = BitsPerPixel = Bmp.bmBitsPixel;
  Info.bmiHeader.biCompression = BI_RGB;
  Info.bmiHeader.biSizeImage = ((width * Bmp.bmBitsPixel + 31) / 32) * 4 * height;

  // TODO make this more efficient...
  last_image.resize(Info.bmiHeader.biSizeImage);
  GetDIBits(DC, last_handle_, 0, height, &last_image[0], &Info, DIB_RGB_COLORS);

  SelectObject(DC, OldBitmap);
  DeleteDC(DC);
}

int Recorder::getEncoderClsid_(const WCHAR* format, CLSID* pClsid)
{
  UINT num = 0;
  UINT size = 0;
  Gdiplus::ImageCodecInfo* pImageCodecInfo;
  Gdiplus::GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;
  pImageCodecInfo = static_cast<Gdiplus::ImageCodecInfo*>(malloc(size));
  if (pImageCodecInfo == nullptr)
    return -1;
  GetImageEncoders(num, size, pImageCodecInfo);
  for (UINT j = 0; j < num; ++j)
  {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;
    }
  }
  free(pImageCodecInfo);
  return -1;
}

void testPixels(std::vector<uint8_t>& v)
{
  std::cout << "###### Output of some image pixels" << std::endl;
  std::cout << "Size: " << v.size() << std::endl;
  std::cout << "First:" << std::endl;
  testPixel(v, 0);
  std::cout << "Second:" << std::endl;
  testPixel(v, 1);
  std::cout << "###### End output." << std::endl;
}

void testPixel(std::vector<uint8_t>& v, int index)
{
  index = index * 4;
  std::bitset<8> first_b(v[index + 0]);
  std::bitset<8> first_g(v[index + 1]);
  std::bitset<8> first_r(v[index + 2]);
  std::bitset<8> first_null(v[index + 3]);
  std::cout << "- B:    " << first_b << " "
    << unsigned(v[index + 0]) << " " << reinterpret_cast<const char*>(&v[0])[index + 0] << std::endl;
  std::cout << "- G:    " << first_g << " "
    << unsigned(v[index + 1]) << " " << reinterpret_cast<const char*>(&v[0])[index + 1] << std::endl;
  std::cout << "- R:    " << first_r << " "
    << unsigned(v[index + 2]) << " " << reinterpret_cast<const char*>(&v[0])[index + 2] << std::endl;
  std::cout << "- NULL: " << first_null << " "
    << unsigned(v[index + 3]) << " " << reinterpret_cast<const char*>(&v[0])[index + 3] << std::endl;
}