#include <algorithm>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "bayan.hpp"

using namespace std;
using namespace otus;
namespace fs = std::filesystem;
namespace po = boost::program_options;

int main(int argc, char **argv) {
  try {
    po::positional_options_description positional_description { };
    positional_description.add("targets", -1);

    po::options_description hidden_description { "Hidden options" };
    hidden_description.add_options()
      ("targets",
        po::value<vector<fs::path>>()->multitoken()->zero_tokens(),
        "targets directories");

    po::options_description named_description {
      "Usage: bayan [OPTIONS...] DIRS...\nAllowed options"
    };

    named_description.add_options()
      ("help,h", "give this help list")
      ("exclude,e", "list of directories to exclude")
      ("level,l",
        po::value<int>()->default_value(-1),
        "max depth of scan, negative for not limit")
      ("mask,m",
        po::value<string>()->default_value("*"),
        "mask of files to compare")
      ("min-file,s",
        po::value<long>()->default_value(2),
        "lower limit of files in bytes to compare")
      ("block-size,b",
        po::value<long>()->default_value(1024),
        "default block size in bytes to process")
      ("hash-func,f",
        po::value<int>()->default_value(0),
        "reserved for future usage");

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

    vector<fs::path> targets;
    if (variables["targets"].empty())
      targets = vector<fs::path>({ "./" });
    else
      targets = variables["targets"].as<vector<fs::path>>();

    otus::Bayan bayan { move(targets) };
    bayan.setLevel(variables["level"].as<int>());
    bayan.setMask(variables["mask"].as<string>());
    bayan.setMinFileSize(variables["min-file"].as<long>());
    bayan.setBlockSize(variables["block-size"].as<long>());
    bayan.run();
  } catch (po::error &e) {
    cerr << "Options error: " << e.what() << endl;
    return EXIT_FAILURE;
  } catch (otus::Bayan::Error &e) {
    cerr << "Runtime error: " << e.what() << endl;
    //return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
