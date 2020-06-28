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

  template <size_t block_size>
  class LazyDigest {
    public:
    class Error: public std::runtime_error {
    public:
      Error(std::string const &message): std::runtime_error (message) { }
    };

    LazyDigest(fs::path const &path): path(path) {
      uintmax_t size;
      try {
        size = fs::file_size(path);
      } catch (fs::filesystem_error &e) {
        throw Error(e);
      }
      length = size / block_size + bool(size % block_size);
      digest.reserve(length);
    }

    bool operator ==(LazyDigest &other) {
      // Сверяем размер.
      // Делаем ленивое вычисление.
    }

    bool isCompleted() { return block == length; }

  private:
    fs::path path;
    uintmax_t length;
    std::vector<unsigned> digest { };
    size_t block { 0 };

    bool getNextBlockDigetst() {
      if (isCompleted()) return false;
      std::ifstream file { path };

      if (!file) throw Error("failed to open " + std::string(path));

      auto buf { std::string(block_size, '\0') };
      if (!file.read(&buf[0], block_size))
        throw Error("unexpected end of " + std::string(path));

      boost::crc_32_type result;
      result.process_bytes(buf.c_str(), block_size);

      digest.push_back(result.checksum());
      ++block;
      return true;
    }
  };
}

#endif
