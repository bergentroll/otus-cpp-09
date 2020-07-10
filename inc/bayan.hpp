#ifndef OTUS_BAYAN_HPP
#define OTUS_BAYAN_HPP

#include <filesystem>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <regex>
#include <vector>

#include "lazy_digest.hpp"


namespace otus {
  namespace fs = std::filesystem;

  // TODO Multithreading.
  class Bayan {
  public:
    class Error: public std::runtime_error {
    public:
      Error(std::string const &message): std::runtime_error(message) { }
    };

    Bayan() = delete;

    Bayan(std::vector<fs::path> const &targets, std::string_view hashFuncName):
    targets(targets) {
      for (auto const &path: targets)
        try {
        if (!fs::exists(path))
          throw Error("path \"" + std::string(path) + " does not exist");
        } catch (fs::filesystem_error const &e) {
          throw Error(e.what());
        }
      if (hashFuncName == "crc16")
        digestFunc = make_crc_digest<boost::crc_16_type>;
      else if (hashFuncName == "crc32")
        digestFunc = make_crc_digest<boost::crc_32_type>;
      else
        throw Error("hash function " + std::string(hashFuncName) + " is unkonwn");
    }

    void SetExclude(std::vector<fs::path> const &excludes) {
      std::for_each(
        excludes.begin(),
        excludes.end(),
        [this](auto path) {
          path += fs::path::preferred_separator;
          this->excludes.insert(path);
        });
    }

    void setLevel(int level) { this->level = level; }

    int getLevel() const { return level; }

    void setPatterns(std::vector<std::string> const &patterns) {
      filePatterns = std::vector<std::regex>(patterns.begin(), patterns.end());
    }

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
    std::function<LazyDigest::DigestFunction> digestFunc;
    std::vector<fs::path> targets;
    std::set<fs::path> excludes;
    std::vector<LazyDigest> digests { };
    std::vector<std::vector<fs::path>> duplicates { };
    std::vector<std::regex> filePatterns { };
    int level { -1 };
    size_t minFileSize { 2 };
    size_t blockSize { 1024 };

    bool matchPatterns(std::string sample) const {
      if (filePatterns.size() == 0) return true;

      for (auto const &pattern: filePatterns)
        if (std::regex_match(sample, pattern)) return true;

      return false;
    };


    void appendDigest(fs::path const &path) {
      try {
        digests.push_back(LazyDigest(path, blockSize, digestFunc));
      } catch (LazyDigest::FileError const &e) {
        std::cerr << "File error: " << e.what() << std::endl;
      } catch (std::bad_alloc const &e) {
        std::cerr << "Failed to process " << path << std::endl;
      }
    }

    void processFileEntry(fs::directory_entry const &entry) {
      if (
          entry.is_regular_file() &&
          !entry.is_symlink() &&
          fs::file_size(entry) >= minFileSize &&
          matchPatterns(entry.path().filename()))
        appendDigest(entry);
    }

    void traverse(fs::path const &path) {
      try {
        if (!fs::is_directory(path)) {
          processFileEntry(fs::directory_entry(path));
          return;
        }

        std::error_code error { };
        for (
            fs::recursive_directory_iterator it {
              path, fs::directory_options::skip_permission_denied }, end_it { };
            it != end_it;
            ++it) {
          auto entry { *it };
          if (entry.is_directory()) {
            auto tmpIt { fs::directory_iterator(entry, error) };
            if (error) {
              std::cerr << error.message() << " on " << entry << std::endl;
            }
            auto tmpPath { entry.path() };
            tmpPath += fs::path::preferred_separator;
            if (
                (level >=0 && it.depth() >= level) ||
                excludes.count(tmpPath))
              it.disable_recursion_pending();
          } else {
            processFileEntry(entry);
          }
        }
      } catch (fs::filesystem_error const &e) {
        std::cerr
          << "Filesystem error on " << e.path1()
          << ": " << e.code().message() << std::endl;
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
            std::cerr << "File error: " << e.what() << std::endl;
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
