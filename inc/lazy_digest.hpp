#ifndef OTUS_LAZY_DIGEST_HPP
#define OTUS_LAZY_DIGEST_HPP

#include <boost/crc.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace otus {
  namespace fs = std::filesystem;

  class LazyDigest {
  public:
    class Error: public std::runtime_error {
    public:
      Error(std::string const &message): std::runtime_error (message) { }
    };

    LazyDigest(fs::path const &path, size_t blockSize):
    blockSize(blockSize), path(path) {
      try {
        size = fs::file_size(path);
      } catch (fs::filesystem_error &e) {
        throw Error(e.what());
      }

      length = size / blockSize + bool(size % blockSize);
      digest.reserve(length);
    }

    unsigned at(size_t idx) {
      if (idx > length) throw std::out_of_range(
          "index " + std::to_string(idx) + " is out of range");

      while(idx > block) getNextBlockDigetst();

      return digest[idx];
    }

    bool operator ==(LazyDigest &other) {
      if (size != other.size) return false;
      for (size_t i { }; i < length; ++i) {
        if (at(i) != other.at(i)) return false;
      }
      return true;
    }

    bool isCompleted() const { return block == length; }

    fs::path const &getPath() const { return path; }

  private:
    size_t blockSize;
    fs::path path;
    uintmax_t size;
    uintmax_t length;
    std::vector<unsigned> digest { };
    size_t block { 0 };

    void getNextBlockDigetst() {
      std::ifstream file { path };

      if (!file) throw Error("failed to open " + std::string(path));

      auto buf { std::string(blockSize, '\0') };
      if (!file.read(&buf[0], blockSize))
        throw Error("unexpected end of " + std::string(path));

      boost::crc_32_type result;
      result.process_bytes(buf.c_str(), blockSize);

      digest.push_back(result.checksum());
      ++block;
    }
  };
}

#endif
