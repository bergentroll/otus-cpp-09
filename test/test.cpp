#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "lazy_digest.hpp"

using namespace std;
using namespace otus;
namespace fs = filesystem;

TEST(lazy_digest, equal) {
  fs::path path { "test_file_equal.txt" };
  ofstream f { path };
  for(int i { }; i < 10; ++i)
    f << to_string(i) << " Dolorem ipsum dolor sit amet";
  f.close();

  LazyDigest dig1 { path, 21 };
  LazyDigest dig2 { path, 21 };

  EXPECT_TRUE(dig1.matches(dig2));
}

TEST(lazy_digest, not_equal) {
  fs::path path1 { "test_file_not_equal_1.txt" };
  fs::path path2 { "test_file_not_equal_2.txt" };
  ofstream f1 { path1 };
  ofstream f2 { path2 };
  for(int i { }; i < 10; ++i) {
    f1 << to_string(i) << " Dolorem ipsum dolor sit amet";
    f2 << to_string(i) << " Dolorem ipsum dolor sit amet";
  }
  f1 << '!'; f2 << '?';
  f1.close(); f2.close();

  LazyDigest dig1 { path1, 21 };
  LazyDigest dig2 { path2, 21 };

  EXPECT_FALSE(dig1.matches(dig2));
}

TEST(lazy_digest, different_block_size) {
  fs::path path { "test_file_block_size.txt" };
  ofstream f { path };
  LazyDigest dig1 { path, 20 }, dig2 { path, 21 };
  EXPECT_THROW(dig1.matches(dig2), LazyDigest::BlockSizeError);
}
