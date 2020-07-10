#ifndef OTUS_BAYAN_HPP
#define OTUS_BAYAN_HPP

#include <regex>
#include <set>

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
    Bayan(std::vector<fs::path> const &targets, std::string_view hashFuncName);
    void SetExclude(std::vector<fs::path> const &excludes);
    void run();

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

    bool matchPatterns(std::string sample) const;
    void appendDigest(fs::path const &path);
    void processFileEntry(fs::directory_entry const &entry);
    void traverse(fs::path const &path);
    void resolveDuplicates();
    void printDuplicates();
  };
}

#endif
