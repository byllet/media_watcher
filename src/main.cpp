#include "media_watcher.hpp"
#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

std::unordered_map<std::string, std::string> LoadConfig(const fs::path &path) {
  if (!fs::exists(path)) {
    std::cerr << "Yaml config not found at: " << path << "\n";
    return {};
  }

  YAML::Node config_yaml = YAML::LoadFile(path);

  std::unordered_map<std::string, std::string> extension_category;

  std::cout << "Loaded extensions:\n";
  for (const auto &pair : config_yaml) {
    std::string category = pair.first.as<std::string>();
    std::cout << category << ": ";
    auto extensions = config_yaml[category].as<std::vector<std::string>>();

    for (const auto &extension : extensions) {
      std::cout << extension << " ";
      extension_category[extension] = category;
    }
    std::cout << "\n";
  }
  std::cout << '\n';

  return extension_category;
}

int main(int argc, char **argv) {
  std::string path = getenv("HOME");
  long long interval = 10000;
  bool server = false;
  bool file_saving = true;

  if (argc < 5) {
    std::cout << "\nUsage ./app  <media_directory_path> <time_interval in ms> "
                 "<enable "
                 "server (0 or 1)> <enable file writing (0 or 1)>\n";
  }

  if (argc >= 5) {
    path = argv[1];
    if (!fs::exists(path)) {
      std::cerr << "File not exists " << path << '\n';
      return 1;
    }
    try {
      interval = std::stoll(argv[2]);
      server = std::stoi(argv[3]);
      file_saving = std::stoi(argv[4]);
    } catch (const std::exception &e) {
      std::cerr << "Error parsing arguments: " << e.what() << '\n';
      return 1;
    }
  }

  std::cout << "\nSettings:\n";
  std::cout << "Path: " << path << '\n';
  std::cout << "Time interval: " << interval << '\n';
  std::cout << "Enable server: " << server << '\n';
  std::cout << "File saving: " << file_saving << '\n';
  std::cout << "\n\n";

  auto extension_category = LoadConfig("./configs/extensions.yaml");
  if (extension_category.empty())
    return 1;

  MediaWatcher mw{path, extension_category, interval, server, file_saving};
  mw.Start();

  return 0;
}