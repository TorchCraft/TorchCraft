/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 * STL stream classes for Zstd compression and decompression.
 * Partially inspired by https://github.com/mateidavid/zstr.
 */

#pragma once

#include <cassert>
#include <fstream>
#include <stdexcept>
#include <vector>

// Required to access ZSTD_isFrame()
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif // _MSC_VER

namespace zstd {

// Custom exception for zstd error codes
class exception : public std::exception {
 public:
  explicit exception(int code) : msg_("zstd: ") {
    msg_ += ZSTD_getErrorName(code);
  }

  const char* what() const throw() {
    return msg_.c_str();
  };

 private:
  std::string msg_;
};

inline size_t check(size_t code) {
  if (ZSTD_isError(code)) {
    throw exception(code);
  }
  return code;
}

// Provides stream compression functionality
class cstream {
 public:
  static const int defaultLevel = 5;

  explicit cstream(int level = defaultLevel) : ended_(false) {
    cstr_ = ZSTD_createCStream();
    if (cstr_ == nullptr) {
    }
    check(ZSTD_initCStream(cstr_, level));
  }

  ~cstream() {
    check(ZSTD_freeCStream(cstr_));
  }

  size_t compress(ZSTD_outBuffer* output, ZSTD_inBuffer* input) {
    return check(ZSTD_compressStream(cstr_, output, input));
  }

  size_t end(ZSTD_outBuffer* output) {
    auto ret = check(ZSTD_endStream(cstr_, output));
    ended_ = true;
    return ret;
  }

  bool ended() const {
    return ended_;
  }

 private:
  ZSTD_CStream* cstr_;
  bool ended_;
};

// Provides stream decompression functionality
class dstream {
 public:
  dstream() {
    dstr_ = ZSTD_createDStream();
    if (dstr_ == nullptr) {
    }
    check(ZSTD_initDStream(dstr_));
  }

  ~dstream() {
    check(ZSTD_freeDStream(dstr_));
  }

  size_t decompress(ZSTD_outBuffer* output, ZSTD_inBuffer* input) {
    return check(ZSTD_decompressStream(dstr_, output, input));
  }

 private:
  ZSTD_DStream* dstr_;
};

// Zstd stream for compression. Data is written in a single big frame.
class ostreambuf : public std::streambuf {
 public:
  explicit ostreambuf(std::streambuf* sbuf, int level = cstream::defaultLevel)
      : sbuf_(sbuf), str_(level) {
    inbuf_.resize(ZSTD_CStreamInSize());
    outbuf_.resize(ZSTD_CStreamOutSize());
    inhint_ = inbuf_.size();
    setp(inbuf_.data(), inbuf_.data() + inhint_);
  }

  virtual ~ostreambuf() {
    sync();
  }

  virtual int_type overflow(int_type ch = traits_type::eof()) {
    auto pos = compress(pptr() - pbase());
    if (pos < 0) {
      setp(nullptr, nullptr);
      return traits_type::eof();
    }
    setp(inbuf_.data() + pos, inbuf_.data() + inhint_);
    return ch == traits_type::eof() ? traits_type::eof() : sputc(ch);
  }

  virtual int sync() {
    overflow();
    if (!pptr() || str_.ended()) {
      return -1;
    }

    // Flush and finish the Zstd frame
    ZSTD_outBuffer output = {outbuf_.data(), outbuf_.size(), 0};
    auto ret = str_.end(&output);
    check(ret);
    if (ret) {
      throw std::runtime_error(std::string("zstd: ") + "not fully flushed");
    }
    if (sbuf_->sputn(reinterpret_cast<char*>(output.dst), output.pos) !=
        output.pos) {
      return -1;
    }
    return 0;
  }

 private:
  ssize_t compress(size_t pos) {
    ZSTD_inBuffer input = {inbuf_.data(), pos, 0};
    while (input.pos != input.size) {
      ZSTD_outBuffer output = {outbuf_.data(), outbuf_.size(), 0};
      auto ret = str_.compress(&output, &input);
      check(ret);
      inhint_ = std::min(ret, inbuf_.size());

      if (output.pos > 0 &&
          sbuf_->sputn(reinterpret_cast<char*>(output.dst), output.pos) !=
              output.pos) {
        return -1;
      }
    }

    return 0;
  }

  std::streambuf* sbuf_;
  cstream str_;
  std::vector<char> inbuf_;
  std::vector<char> outbuf_;
  size_t inhint_;
};

// Zstd stream for decompression. If input data is not compressed, this stream
// will simply copy it.
class istreambuf : public std::streambuf {
 public:
  explicit istreambuf(std::streambuf* sbuf)
      : sbuf_(sbuf) {
    inbuf_.resize(ZSTD_DStreamInSize());
    inhint_ = inbuf_.size();
    setg(inbuf_.data(), inbuf_.data(), inbuf_.data());
  }

  virtual std::streambuf::int_type underflow() {
    if (gptr() != egptr()) {
      return traits_type::eof();
    }

    while (true) {
      if (inpos_ >= inavail_) {
        inavail_ = sbuf_->sgetn(inbuf_.data(), inhint_);
        if (inavail_ == 0) {
          return traits_type::eof();
        }
        inpos_ = 0;
      }

      // Check whether data is actually compressed
      if (!detected_) {
        compressed_ = ZSTD_isFrame(inbuf_.data(), inavail_);
        detected_ = true;
        if (compressed_) {
          outbuf_.resize(ZSTD_DStreamOutSize());
        }
      }

      if (compressed_) {
        // Consume input
        ZSTD_inBuffer input = {inbuf_.data(), inavail_, inpos_};
        ZSTD_outBuffer output = {outbuf_.data(), outbuf_.size(), 0};
        auto ret = str_.decompress(&output, &input);
        inhint_ = std::min(ret, inbuf_.size());
        inpos_ = input.pos;
        if (output.pos == 0 && inhint_ > 0 && inpos_ >= inavail_) {
          // Zstd did not decompress anything but requested more data
          continue;
        }
        setg(outbuf_.data(), outbuf_.data(), outbuf_.data() + output.pos);
      } else {
        // Re-use inbuf_ to avoid extra copy
        inpos_ = inavail_;
        setg(inbuf_.data(), inbuf_.data(), inbuf_.data() + inavail_);
      }

      break;
    }
    return traits_type::to_int_type(*gptr());
  }

 private:
  std::streambuf* sbuf_;
  dstream str_;
  std::vector<char> inbuf_;
  std::vector<char> outbuf_; // only needed if actually compressed
  size_t inhint_;
  size_t inpos_ = 0;
  size_t inavail_ = 0;
  bool detected_ = false;
  bool compressed_ = false;
};

// This class enables [io]fstream below to inherit from [io]stream (required for
// setting a custom streambuf) while still constructing a corresponding
// [io]fstream first (required for initializing the Zstd streambufs).
template <typename T>
class fsholder {
 public:
  explicit fsholder(
      const std::string& path,
      std::ios_base::openmode mode = std::ios_base::out)
      : fs_(path, mode) {}

 protected:
  T fs_;
};

// Output file stream that writes Zstd-compressed data
class ofstream : private fsholder<std::ofstream>, public std::ostream {
 public:
  explicit ofstream(
      const std::string& path,
      std::ios_base::openmode mode = std::ios_base::out)
      : fsholder<std::ofstream>(path, mode | std::ios_base::binary),
        std::ostream(new ostreambuf(fs_.rdbuf())) {
    exceptions(std::ios_base::badbit);
  }

  virtual ~ofstream() {
    if (rdbuf()) {
      delete rdbuf();
    }
  }

  virtual operator bool() const {
    return bool(fs_);
  }

  void close() {
    flush();
    fs_.close();
  }
};

// Input file stream for Zstd-compressed data
class ifstream : private fsholder<std::ifstream>, public std::istream {
 public:
  explicit ifstream(
      const std::string& path,
      std::ios_base::openmode mode = std::ios_base::in)
      : fsholder<std::ifstream>(path, mode | std::ios_base::binary),
        std::istream(new istreambuf(fs_.rdbuf())) {
    exceptions(std::ios_base::badbit);
  }

  virtual ~ifstream() {
    if (rdbuf()) {
      delete rdbuf();
    }
  }

  operator bool() const {
    return bool(fs_);
  }

  void close() {
    fs_.close();
  }
};

} // namespace zstd
