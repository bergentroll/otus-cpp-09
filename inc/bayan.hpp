#ifndef OTUS_BAYAN_HPP
#define OTUS_BAYAN_HPP

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

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
      // Ищем файлы.
      for (auto const &path: targets) traverse(path);
      // Сравниваем хэши.
      // Выводим дубли.
    }

  private:
    std::vector<fs::path> targets;
    int level { -1 };
    std::string mask { "*" };
    size_t minFileSize { 2 };
    size_t blockSize { 1024 };
    std::vector<fs::path> files;

    void traverse(fs::path const &path) {
      if (fs::is_regular_file(path)) {
        files.push_back(path);
        return;
      }

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
            files.push_back(entry);
          }
        }
      } catch (fs::filesystem_error &e) {
        throw Error(e.what());
      }
    }

    void findCollisions() {
    }
  };
}

#endif
