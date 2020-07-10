/* To the extent possible under law, bayan has waived all copyright and related
 * or neighboring rights to program.
 *
 * Written by 2020 Anton Karmanov <bergentroll@insiberia.net>.
 */

#include <fstream>
#include <stdexcept>
#include <string>

#include "lazy_digest.hpp"

using namespace std;
using namespace otus;

LazyDigest::LazyDigest(fs::path const &path, size_t blockSize, function<DigestFunction> func):
blockSize(blockSize), path(path), makeDigest(func) {
  try {
    size = fs::file_size(path);
  } catch (fs::filesystem_error &e) {
    throw FileError(e.what(), path);
  }

  lengthInBlocks = size / blockSize + bool(size % blockSize);
  digest.reserve(lengthInBlocks);
}

unsigned LazyDigest::at(size_t idx) {
  if (idx >= lengthInBlocks) throw out_of_range(
      "index " + to_string(idx) + " is out of range");

  while (idx >= block) getNextBlockDigest();

  return digest[idx];
}

bool LazyDigest::forward() {
  if (isCompleted()) return false;
  getNextBlockDigest();
  return true;
}

bool LazyDigest::matches(LazyDigest &other) {
  if (blockSize != other.blockSize)
    throw BinaryOpIncompatibility(
        "comparing LazyDigest objects with different blocksize");
  if (*makeDigest.target<DigestFunction*>() != *other.makeDigest.target<DigestFunction*>()) {
    throw BinaryOpIncompatibility(
        "comparing LazyDigest objects with different digest function");
  }

  if (size != other.size) return false;
  for (size_t i { }; i < lengthInBlocks; ++i) {
    if (at(i) != other.at(i)) return false;
  }
  return true;
}

LazyDigest::operator string() const {
  stringstream ss { };
  ss << hex;
  for (size_t i { }; i < lengthInBlocks; ++i) {
    if (i > 0) ss << '-';
    if (i < block) ss << digest[i];
    else ss << "???";
  }
  return ss.str();
}

void LazyDigest::getNextBlockDigest() {
  ifstream file { path };
  if (!file) throw FileError("failed to open " + string(path), path);
  file.seekg(block * blockSize);

  auto buf { string(blockSize, '\0') };
  if (!file.read(&buf[0], blockSize) && block < lengthInBlocks - 1)
    throw FileError("unexpected end of " + string(path), path);

  digest.push_back(makeDigest(buf));
  ++block;
}
