#pragma once

#include "BS_thread_pool.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

class DirectoryWalker {
public:
  DirectoryWalker(
      const std::string &path,
      const std::unordered_map<std::string, std::string> &extension_category);

  std::unordered_map<std::string, std::vector<std::string>>
  GetFilenamesDevidedByCategory();

private:
  void RecursiveFileSearch(const fs::path &path);
  void AddFile(const fs::path &path);
  void SubmitTask(const fs::path &path);

private:
  std::string m_path;
  std::unordered_map<std::string, std::string> m_extension_category;
  std::unordered_map<std::string, std::vector<std::string>> m_category_files;

private:
  BS::thread_pool<BS::tp::none> m_pool;
  std::atomic<ssize_t> m_task_counter;

  std::mutex m_files_mutex;
  std::mutex m_wait_mutex;
  std::condition_variable wait_cv;
};