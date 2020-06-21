#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char **argv) {
  try {
    po::positional_options_description positional_description { };
    positional_description.add("target", -1);

    po::options_description hidden_description { "Hidden options" };
    hidden_description.add_options()
      ("target", po::value<vector<string>>(),"target directories");

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
        po::value<int>()->default_value(2),
        "lower limit of files in bytes to compare")
      ("block-size,b",
        po::value<int>()->default_value(1024),
        "default block size in bytes to process")
      ("hash-func,f",
        po::value<int>()->default_value(0),
        "reserved for future usage");

    po::variables_map variables;
    po::store(
      po::command_line_parser(argc, argv)
        .options(named_description)
        .options(hidden_description)
        .positional(positional_description)
        .run(),
      variables);
    po::notify(variables);

    if (variables.count("help")) std::cout << named_description<< endl;
  } catch (po::unknown_option &e) {
    cerr << e.what() << endl;
  }

  return EXIT_SUCCESS;
}
