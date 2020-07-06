#ifndef OTUS_LAZY_DIGEST_HPP
#define OTUS_LAZY_DIGEST_HPP

#include <boost/crc.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>


namespace otus {
  namespace fs = std::filesystem;

  class LazyDigest {
  public:
    class FileError: public std::runtime_error {
    public:
      FileError(std::string const &message, fs::path const &path):
      std::runtime_error (message), path(path) { }

      // Path helps to determine failed LazyDigest on binary operations.
      fs::path getPath() const { return path; }

    private:
      fs::path const path;
    };

    class BlockSizeError: public std::domain_error {
      public:
      BlockSizeError():
      std::domain_error("comparing LazyDigest objects with different blocksize") { }

      BlockSizeError(std::string const &message): std::domain_error(message) { }
    };

    LazyDigest(fs::path const &path, size_t blockSize):
    blockSize(blockSize), path(path) {
      try {
        size = fs::file_size(path);
      } catch (fs::filesystem_error &e) {
        throw FileError(e.what(), path);
      }

      lengthInBlocks = size / blockSize + bool(size % blockSize);
      digest.reserve(lengthInBlocks);
    }

    unsigned at(size_t idx) {
      if (idx >= lengthInBlocks) throw std::out_of_range(
          "index " + std::to_string(idx) + " is out of range");

      while (idx >= block) getNextBlockDigest();

      return digest[idx];
    }

    bool forward() {
      if (isCompleted()) return false;
      getNextBlockDigest();
      return true;
    }

    void completeNow() {
      while (forward());
    }

    bool matches(LazyDigest &other) {
      if (blockSize != other.blockSize)
        throw BlockSizeError();

      if (size != other.size) return false;
      for (size_t i { }; i < lengthInBlocks; ++i) {
        if (at(i) != other.at(i)) return false;
      }
      return true;
    }

    bool isCompleted() const {
      return block == lengthInBlocks; }

    fs::path const &getPath() const { return path; }

    operator std::string() const {
      std::stringstream ss { };
      ss << std::hex;
      for (size_t i { }; i < lengthInBlocks; ++i) {
        if (i > 0) ss << '-';
        if (i < block) ss << digest[i];
        else ss << "???";
      }
      return ss.str();
    }

  private:
    size_t blockSize;
    fs::path path;
    uintmax_t size;
    uintmax_t lengthInBlocks;
    std::vector<unsigned> digest { };
    size_t block { 0 };

    void getNextBlockDigest() {
      std::ifstream file { path };
      if (!file) throw FileError("failed to open " + std::string(path), path);
      file.seekg(block * blockSize);

      auto buf { std::string(blockSize, '\0') };
      if (!file.read(&buf[0], blockSize) && block < lengthInBlocks - 1)
        throw FileError("unexpected end of " + std::string(path), path);

      boost::crc_32_type result;
      result.process_bytes(buf.c_str(), blockSize);

      digest.push_back(result.checksum());
      ++block;
    }
  };


  std::ostream & operator<< (std::ostream &stream, LazyDigest const &digest) {
    stream << std::string(digest);
    return stream;
  }
}

#endif
