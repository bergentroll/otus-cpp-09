/* To the extent possible under law, bayan has waived all copyright and related
 * or neighboring rights to program.
 *
 * Written by 2020 Anton Karmanov <bergentroll@insiberia.net>.
 */

#ifndef OTUS_LAZY_DIGEST_HPP
#define OTUS_LAZY_DIGEST_HPP

#include <boost/crc.hpp>
#include <filesystem>
#include <functional>

namespace otus {
  namespace fs = std::filesystem;

  template <typename T>
  unsigned make_crc_digest(std::string const &input) {
    T result;
    result.process_bytes(input.c_str(), input.size());
    return result.checksum();
  }

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

    class BinaryOpIncompatibility: public std::domain_error {
      public:
      BinaryOpIncompatibility(std::string const &message): std::domain_error(message) { }
    };

    using DigestFunction = unsigned (std::string const &);

    LazyDigest(
      fs::path const &path,
      size_t blockSize,
      std::function<DigestFunction> func = make_crc_digest<boost::crc_32_type>);

    unsigned at(size_t idx);

    bool forward();

    void completeNow() { while (forward()); }

    bool matches(LazyDigest &other);

    bool isCompleted() const { return block == lengthInBlocks; }

    fs::path const &getPath() const { return path; }

    operator std::string() const;

  private:
    size_t blockSize;
    fs::path path;
    uintmax_t size;
    uintmax_t lengthInBlocks;
    std::vector<unsigned> digest { };
    size_t block { 0 };
    std::function<DigestFunction> makeDigest;

    void getNextBlockDigest();
  };


  inline std::ostream & operator<< (std::ostream &stream, LazyDigest const &digest) {
    stream << std::string(digest);
    return stream;
  }
}

#endif
