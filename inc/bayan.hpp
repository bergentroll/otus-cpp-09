#ifndef OTUS_BAYAN_HPP
#define OTUS_BAYAN_HPP

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include <system_error>

#include <fstream>
#include <boost/crc.hpp>
#include <unordered_map>

// TODO Delete
#include <iostream>

namespace otus {
  namespace fs = std::filesystem;

  class Bayan {
  public:
    class Error: public std::runtime_error {
    public:
      Error(std::string const &message): std::runtime_error(message) { }
    };

    Bayan() {
      targets.push_back("./");
    }

    Bayan(std::vector<fs::path> const &targets): targets(targets) {
      for (auto const &path: targets)
        if (!fs::exists(path))
          throw Error("path \"" + std::string(path) + " does not exist");
      if (this->targets.empty()) Bayan();
    }

    void setLevel(int level) { }
    int getLevel() const { return level; }

    void setMask(std::string mask) {
      this->mask = mask;
    }

    std::string getMask() const { return mask; }

    void setMinFileSize(long fileSize) {
      if (fileSize < 0) throw Error("minimal file size can not be negative");
      minFileSize = fileSize;
    }

    size_t getMinFileSize() { return minFileSize; }

    void setBlockSize(long blockSize) {
      if (blockSize <= 0) throw Error("block size can not be negative or zero");
      this->blockSize = blockSize;
    }

    size_t getBlockSize() { return blockSize; }

    void run() {
      for (auto const &path: targets) {
        traverse(path);
      }
      findCollisions();
    }

  private:
    std::vector<fs::path> targets;
    int level { -1 };
    std::string mask { "*" };
    size_t minFileSize { 2 };
    size_t blockSize { 1024 };
    std::unordered_map<std::string, unsigned> digests;

    void traverse(fs::path const &path) {
      if (fs::is_regular_file(path))
        return makeDigest(path);

      try {
        std::error_code error { };
        for (
            fs::recursive_directory_iterator it {
              path, fs::directory_options::skip_permission_denied }, end_it { };
            it != end_it;
            ++it) {
          auto entry { *it };
          if (entry.is_directory()) {
            auto tmp { fs::directory_iterator(entry, error) };
            if (error) {
              std::cerr << error.message() << " on " << entry << std::endl;
              continue;
            }
          } else if (entry.is_regular_file() && !entry.is_symlink()) {
            makeDigest(entry);
          }
        }
      } catch (fs::filesystem_error &e) {
        throw Error(e.what());
      }
    }

    void makeDigest(fs::path const &path) {
      //std::cout << path << std::endl;
      std::ifstream file { path };
      if (!file) {
        std::cerr << "Failed to open " << path << std::endl;
        return;
      }

      boost::crc_32_type result;
      do {
        char buffer[100];
        file.read(buffer, 100);
        result.process_bytes(buffer, file.gcount());
      } while (file);

      digests[path] = result.checksum();

      //std::cout << result.checksum() << std::endl;
    }

    void findCollisions() {
      for (auto [path, digest]: digests) {
        for (auto [path_other, digest_other]: digests) {
          if (path != path_other && digest == digest_other) {
            std::cout << path << " is duplicate of " << path_other << std::endl;
          }
        }
      }
    }
  };
}

#endif
