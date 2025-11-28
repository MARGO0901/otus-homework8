#include "functions.h"

#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fnmatch.h>

using boost::filesystem::recursive_directory_iterator;

std::vector<boost::filesystem::path> fillingVector(const scanConfig &config){

  std::vector<boost::filesystem::path> files;

  for (const auto &dir : config.directories) {
    try {
      recursive_directory_iterator it(dir);
      boost::filesystem::recursive_directory_iterator end;

      while (it != end) {
        if (it.depth() > config.scan_level) {
          // выйти из текущей директории и поднять на уровень выше
          it.pop();
          continue;
        }
        // если это файл с размером больше указанного в опциях - добавить к
        // вектору
        if (boost::filesystem::is_regular_file(it.status())) {
          auto file_size = boost::filesystem::file_size(it->path());
          if (file_size >= config.min_file_size) {
            //если название файла удовлетворяет маске
            if ((!config.mask.empty() && fnmatch(config.mask.c_str(), it->path().filename().c_str(), 0) == 0) || (config.mask.empty())) {
              files.push_back(it->path().string());
            }            
          }
        }
        // если это директория из списка исключений - по
        else if (boost::filesystem::is_directory(it->status())) {
          if (std::find(config.ex_directories.begin(), config.ex_directories.end(),
                        it->path()) != config.ex_directories.end()) {
            // пропустить эту директорию
            it.disable_recursion_pending();
          }
        }
        boost::system::error_code ec;
        it.increment(ec);
        if (ec) {
          std::cerr << "Ошибка доступа: " << ec.message() << " - " << it->path()
                    << std::endl;
          it.pop();
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "Ошибка: " << e.what() << std::endl;
    }
  }
  return files;
}


unsigned int hashCRC32(std::vector<char> &buffer) {
  boost::crc_32_type crc;
  crc.process_bytes(buffer.data(), buffer.size());
  unsigned int hash = crc.checksum();
  return hash;
}


unsigned int hashMD5 (std::vector<char> &buffer) {
    boost::uuids::detail::md5 md5;
    md5.process_bytes(buffer.data(), buffer.size());
    boost::uuids::detail::md5::digest_type digest;
    md5.get_digest(digest);
    return digest[0]^digest[1]^digest[2]^digest[3];
}


unsigned int hashSHA1(std::vector<char> &buffer) {
  boost::uuids::detail::sha1 sha1;
  sha1.process_bytes(buffer.data(), buffer.size());
  boost::uuids::detail::sha1::digest_type digest;
  sha1.get_digest(digest);
  return digest[0]^digest[1]^digest[2]^digest[3];
}



bool compareFiles(boost::filesystem::path &path_file1,
                   boost::filesystem::path path_file2, int size, std::string &hash) {

  boost::iostreams::file_source file1(path_file1.string());
  boost::iostreams::file_source file2(path_file2.string());

  if (!file1.is_open() || !file2.is_open()) {
    if (!file1.is_open())
      std::cerr << "Не удалось открыть файл " << path_file1.string()
                << std::endl;
    if (!file2.is_open())
      std::cerr << "Не удалось открыть файл " << path_file2.string()
                << std::endl;
    return false;
  }

  std::vector<char> buffer1(size), buffer2(size);
  std::streamsize bytes_read1, bytes_read2;

  while (true) {
    bytes_read1 = file1.read(buffer1.data(), buffer1.size());
    bytes_read2 = file2.read(buffer2.data(), buffer2.size());

    if (bytes_read1 != bytes_read2) return false;

    if (bytes_read1 <= 0)
      break;
    // заполнение нулями
    while (bytes_read1 < size) {
      buffer1[bytes_read1] = '\0';
      bytes_read1++;
    }

    if (bytes_read2 <= 0)
      break;
    // заполнение нулями
    while (bytes_read2 < size) {
      buffer2[bytes_read2] = '\0';
      bytes_read2++;
    }

    unsigned int hash1;
    unsigned int hash2;

    if (boost::iequals(hash,"crc32")) {
      hash1 = hashCRC32(buffer1);
      hash2 = hashCRC32(buffer2);
    }
    else if (boost::iequals(hash, "md5")) {
      hash1 = hashMD5(buffer1);
      hash2 = hashMD5(buffer2);
    }
    else if (boost::iequals(hash, "sh1")) {
      hash1 = hashSHA1(buffer1);
      hash2 = hashSHA1(buffer2);
    }

    if (hash1 != hash2)
      return false;
  }

  return true;
}