#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "bayan.hpp"

using namespace std;
using namespace otus;

Bayan::Bayan(vector<fs::path> const &targets, string_view hashFuncName):
targets(targets) {
  for (auto const &path: targets)
    try {
      if (!fs::exists(path))
        throw Error("path \"" + string(path) + " does not exist");
    } catch (fs::filesystem_error const &e) {
      throw Error(e.what());
    }
  if (hashFuncName == "crc16")
    digestFunc = make_crc_digest<boost::crc_16_type>;
  else if (hashFuncName == "crc32")
    digestFunc = make_crc_digest<boost::crc_32_type>;
  else
    throw Error("hash function " + string(hashFuncName) + " is unkonwn");
}

void Bayan::SetExclude(vector<fs::path> const &excludes) {
  for_each(
    excludes.begin(),
    excludes.end(),
    [this](auto path) {
      path += fs::path::preferred_separator;
      this->excludes.insert(path);
    });
}

void Bayan::run() {
  for (auto const &path: targets) traverse(path);

  resolveDuplicates();
  printDuplicates();
}

bool Bayan::matchPatterns(string sample) const {
  if (filePatterns.size() == 0) return true;

  for (auto const &pattern: filePatterns)
    if (regex_match(sample, pattern)) return true;

  return false;
};

void Bayan::appendDigest(fs::path const &path) {
  try {
    digests.push_back(LazyDigest(path, blockSize, digestFunc));
  } catch (LazyDigest::FileError const &e) {
    cerr << "File error: " << e.what() << endl;
  } catch (bad_alloc const &e) {
    cerr << "Failed to process " << path << endl;
  }
}

void Bayan::processFileEntry(fs::directory_entry const &entry) {
  if (
      entry.is_regular_file() &&
      !entry.is_symlink() &&
      fs::file_size(entry) >= minFileSize &&
      matchPatterns(entry.path().filename()))
    appendDigest(entry);
}

void Bayan::traverse(fs::path const &path) {
  try {
    if (!fs::is_directory(path)) {
      processFileEntry(fs::directory_entry(path));
      return;
    }

    error_code error { };
    for (
        fs::recursive_directory_iterator it {
          path, fs::directory_options::skip_permission_denied }, end_it { };
        it != end_it;
        ++it) {
      auto entry { *it };
      if (entry.is_directory()) {
        // To catch permission issues.
        auto tmpIt { fs::directory_iterator(entry, error) };
        if (error) {
          cerr << error.message() << " on " << entry << endl;
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
    cerr
      << "Filesystem error on " << e.path1()
      << ": " << e.code().message() << endl;
  }
}

void Bayan::resolveDuplicates() {
  set<fs::path> skip { };

  for (auto it1 { digests.begin() }; it1 != digests.end(); ++it1) {
    auto &digest1 { *it1 };
    auto &path1 { digest1.getPath() };
    if (skip.count(path1)) continue;

    bool firstDup { true };

    for (auto it2 { next(it1) }; it2 != digests.end() ; ++it2) {
      auto &digest2 { *it2 };
      auto &path2 { digest2.getPath() };
      if (skip.count(path2)) continue;

      bool match;
      try {
        match = digest1.matches(digest2);
      } catch (LazyDigest::FileError const &e) {
        cerr << "File error: " << e.what() << endl;
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

void Bayan::printDuplicates() {
  for (auto const &group: duplicates) {
    cout << endl;
    for (auto const &path: group) {
      cout << path << endl;
    }
  }
}
