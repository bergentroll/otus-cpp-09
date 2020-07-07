#ifndef OTUS_BAYAN_HPP
#define OTUS_BAYAN_HPP

#include <filesystem>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "lazy_digest.hpp"


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

    void SetExclude(std::vector<fs::path> const &excludes) {
      this->excludes = std::set<fs::path>(excludes.begin(), excludes.end());
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
      for (auto const &path: targets) traverse(path);

      resolveDuplicates();
      printDuplicates();
    }

  private:
    std::vector<fs::path> targets;
    std::set<fs::path> excludes;
    std::vector<LazyDigest> digests { };
    std::vector<std::vector<fs::path>> duplicates { };
    int level { -1 };
    std::string mask { "*" };
    size_t minFileSize { 2 };
    size_t blockSize { 1024 };

    void appendDigest(fs::path const &path) {
      try {
        digests.push_back(LazyDigest(path, blockSize));
      } catch (LazyDigest::FileError const &e) {
        std::cerr << e.what() << std::endl;
      } catch (std::bad_alloc const &e) {
        std::cerr << "Failed to process " << path << std::endl;
      }
    }

    void traverse(fs::path const &path) {
      if (fs::is_regular_file(path)) {
        try {
          if (fs::file_size(path) >= minFileSize) appendDigest(path);
        } catch (fs::filesystem_error const &e) {
          std::cerr << e.what() << std::endl;
        }
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
            if (excludes.count(entry)) {
              it.disable_recursion_pending();
            }
            if (error) {
              std::cerr << error.message() << " on " << entry << std::endl;
            }
          } else if (
              entry.is_regular_file() &&
              !entry.is_symlink() &&
              fs::file_size(entry) >= minFileSize) {
            appendDigest(entry);
          }
        }
      } catch (fs::filesystem_error const &e) {
        throw Error(e.what());
      }
    }

    void resolveDuplicates() {
      std::set<fs::path> skip { };

      for (auto it1 { digests.begin() }; it1 != digests.end(); ++it1) {
        auto &digest1 { *it1 };
        auto &path1 { digest1.getPath() };
        if (skip.count(path1)) continue;

        bool firstDup { true };

        for (auto it2 { std::next(it1) }; it2 != digests.end() ; ++it2) {
          auto &digest2 { *it2 };
          auto &path2 { digest2.getPath() };
          if (skip.count(path2)) continue;

          bool match;
          try {
            match = digest1.matches(digest2);
          } catch (LazyDigest::FileError const &e) {
            std::cerr << e.what() << std::endl;
            skip.insert(e.getPath());
          }

          if (match) {
            if (firstDup) {
              duplicates.push_back({ path1 });
              firstDup = false;
            }
            duplicates.back().push_back(path2);
            skip.insert(path2);
          }
        }
      }
    }

    void printDuplicates() {
      for (auto const &group: duplicates) {
        std::cout << std::endl;
        for (auto const &path: group) {
          std::cout << path << std::endl;
        }
      }
    }

  };
}

#endif
