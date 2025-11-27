#include <boost/algorithm/string/predicate.hpp>
#include <boost/program_options.hpp>

#include <iostream>

#include "functions.h"

int main(int argc, char **argv) {

  scanConfig config;
  int block_size = 5;
  std::string hash = "crc32";

  //---------------------------Описание опций-----------------------
  try {
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "Help message")
      ("directory,D", po::value<std::vector<std::string>>()->multitoken(),"Directory to scan")
      ("exclude_directory,E",po::value<std::vector<std::string>>()->multitoken(), "Directory to exclude")
      ("scan_level,L", po::value<int>(), "Level of scan")
      ("min_file_size,s", po::value<int>(), "Minimum file size")
      ("mask_file_name,M", po::value<std::string>(),"Mask of file name")
      ("hash,H", po::value<std::string>(), "Hash algorithm")
      ("block_size,S", po::value<int>(), "Size_of_block");

    // парсинг командной строки
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") || vm.count("h")) {
      std::cout << desc << std::endl;
    }
    if (vm.count("directory") || vm.count("D")) {
      config.directories = vm["directory"].as<std::vector<std::string>>();
    }
    if (vm.count("exclude_directory") || vm.count("E")) {
      config.ex_directories = vm["exclude_directory"].as<std::vector<std::string>>();
    }
    if (vm.count("scan_level") || vm.count("L")) {
      config.scan_level = vm["scan_level"].as<int>();
    }
    if (vm.count("min_file_size") || vm.count("s")) {
      config.min_file_size = vm["min_file_size"].as<int>();
    }
    if (vm.count("mask_file_name") || vm.count("M")) {
      config.mask = vm["mask_file_name"].as<std::string>();
    }
    if (vm.count("block_size") || vm.count("S")) {
      block_size = vm["block_size"].as<int>(); 
    }
    if (vm.count("hash") || vm.count("H")) {
      hash = vm["hash"].as<std::string>();
    }
  } catch (std::exception &e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
  }

  std::vector<boost::filesystem::path> files;
  std::vector<boost::filesystem::path> touch_files;

  files = fillingVector(config);

  for (size_t i = 0; i < files.size(); ++i) {
    std::string str = files[i].string() + "\n";
    int count_compare = 0;

    auto it = std::find(touch_files.begin(), touch_files.end(), files[i]);
    if (it != touch_files.end())
      continue;

    for (size_t j = i + 1; j < files.size(); ++j) {

      auto it = std::find(touch_files.begin(), touch_files.end(), files[j]);
      if (it != touch_files.end())
        continue;

      bool result = compareFiles(files[i], files[j], block_size, hash);
      if (result) {
        str += files[j].string() + "\n";
        touch_files.push_back(files[j]);
        count_compare++;
      }
    }
    if (count_compare > 0)
      std::cout << str << std::endl;
  }

  return 0;
}