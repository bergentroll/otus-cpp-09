#include <algorithm>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "bayan.hpp"

using namespace std;
using namespace otus;
namespace fs = std::filesystem;
namespace po = boost::program_options;

namespace std {
  template <typename T>
  ostream& operator<<(ostream &os, const vector<T> &vec) {
    for (auto it { vec.cbegin() }; it != vec.cend(); ++it) {
      if (it != vec.cbegin()) os << ' ';
      os << *it;
    }
    return os;
  }
} 

int main(int argc, char **argv) {
  try {
    po::positional_options_description positional_description { };
    positional_description.add("targets", -1);

    po::options_description hidden_description { "Hidden options" };
    hidden_description.add_options()
      ("targets",
        po::value<vector<fs::path>>()->
          multitoken()->value_name("PATH")->default_value({ "./" }),
        "targets directories");

    po::options_description named_description {
      "Allowed options"
    };

    named_description.add_options()
      ("help,h", "give this help list")
      ("exclude,e",
        po::value<vector<fs::path>>()->
          multitoken()->value_name("DIR")->default_value({ }),
        "list of directories to exclude, may be passed multiple times")
      ("level,l",
        po::value<int>()->value_name("INT")->default_value(-1),
        "max depth of scan, negative for no limit")
      ("pattern,p",
        po::value<vector<string>>()->
          multitoken()->value_name("REGEX")->default_value({ }),
        "only matched files will be processed when given, may be passed multiple times")
      ("min-file,s",
        po::value<long>()->value_name("BYTES")->default_value(2),
        "lower limit of files in bytes to compare")
      ("block-size,b",
        po::value<long>()->value_name("BYTES")->default_value(1024),
        "default block size in bytes to process")
      ("hash-func,f",
        po::value<string>()->value_name("NAME")->default_value("crc32"),
        "hash function to use, crc16, crc32 is valid");

    po::options_description cmdline_description { };
    cmdline_description.add(hidden_description).add(named_description);

    po::variables_map variables;
    po::store(
      po::command_line_parser(argc, argv)
        .options(cmdline_description)
        .positional(positional_description)
        .run(),
      variables);
    po::notify(variables);

    if (variables.count("help")) {
      std::cout << named_description<< endl;
      return EXIT_SUCCESS;
    }

    otus::Bayan bayan {
      variables["targets"].as<vector<fs::path>>(),
      variables["hash-func"].as<string>() };
    bayan.SetExclude(variables["exclude"].as<vector<fs::path>>());
    bayan.setLevel(variables["level"].as<int>());
    bayan.setPatterns(variables["pattern"].as<vector<string>>());
    bayan.setMinFileSize(variables["min-file"].as<long>());
    bayan.setBlockSize(variables["block-size"].as<long>());

    variables.clear();

    bayan.run();
  } catch (po::error const &e) {
    cerr << "Options error: " << e.what() << endl;
    return EXIT_FAILURE;
  } catch (otus::Bayan::Error const &e) {
    cerr << "Runtime error: " << e.what() << endl;
  }

  return EXIT_SUCCESS;
}
