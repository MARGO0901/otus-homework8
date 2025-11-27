#pragma once

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

struct scanConfig {
    std::vector<std::string> directories{"."};
    std::vector<std::string> ex_directories;
    int scan_level = 0;
    int min_file_size = 0;
    std::string mask;
};

std::vector<boost::filesystem::path> fillingVector(const scanConfig &config);

unsigned int hashCRC32(std::vector<char> &buffer);
unsigned int hashMD5 (std::vector<char> &buffer);
unsigned int hashSHA1(std::vector<char> &buffer);

bool compareFiles(boost::filesystem::path &path_file1,
                   boost::filesystem::path path_file2, int size, std::string &hash);
