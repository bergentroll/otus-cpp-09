#ifndef OTUS_BAYAN_HPP
#define OTUS_BAYAN_HPP

#include <stdexcept>
#include <string>
#include <vector>

namespace otus {
  class Bayan {
  public:
    class Error: public std::runtime_error {
    public:
      Error(std::string const &message): std::runtime_error(message) { }
    };

    Bayan() {
      targets.push_back("./");
    }

    Bayan(std::vector<std::string> const &targets): targets(targets) {
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
        process(path);
      }
    }

  private:
    std::vector<std::string> targets;
    int level { -1 };
    std::string mask { "*" };
    size_t minFileSize { 2 };
    size_t blockSize { 1024 };

    void process(std::string const &) { }
  };
}

#endif
