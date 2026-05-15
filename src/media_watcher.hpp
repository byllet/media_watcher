#pragma once

#include "BS_thread_pool.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

class MediaWatcher {
public:
  MediaWatcher(
      const std::unordered_map<std::string, std::string> &extension_category,
      long long interval, const std::string &path, bool server,
      bool file_saving)
      : m_extension_category{extension_category}, m_interval{interval},
        m_path{path}, m_server{server}, m_file_saving{file_saving}, m_pool{},
        m_task_counter{0} {}

  void Start();

private:
  void RecursiveFileSearch(const fs::path &path);
  void AddFile(const fs::path &path);
  void SubmitTask(const fs::path &path);
  void Iteraion();
  void StartServer();

private:
  std::unordered_map<std::string, std::string> m_extension_category;
  long long m_interval;
  std::string m_path;
  std::unordered_map<std::string, std::vector<std::string>> category_files;
  bool m_server;
  bool m_file_saving;

private:
  BS::thread_pool<BS::tp::none> m_pool;
  std::atomic<long long> m_task_counter;

  std::mutex m_files_mutex;
  std::mutex m_wait_mutex;
  std::condition_variable wait_cv;

private:
  nlohmann::json m_cached_json;
  std::mutex m_json_mutex;
};