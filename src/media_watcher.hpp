#pragma once

#include "directory_walker.hpp"
#include "server.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

class MediaWatcher {
public:
  MediaWatcher(
      const std::string &path,
      const std::unordered_map<std::string, std::string> &extension_category,
      long long interval, bool server, bool file_saving);

  void Start();

private:
  void Iteraion();

private:
  DirectoryWalker m_dw;
  Server m_srv;

  long long m_interval;
  std::string m_path;

  bool m_run_server;
  bool m_file_saving;

private:
  nlohmann::json m_cached_json;
};