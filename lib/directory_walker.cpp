#include "directory_walker.hpp"
#include <unordered_set>

const std::unordered_set<std::string> skip = {"Library"};

DirectoryWalker::DirectoryWalker(
    const std::string &path,
    const std::unordered_map<std::string, std::string> &extension_category)
    : m_path{path}, m_extension_category{extension_category},
      m_task_counter{0} {}

void DirectoryWalker::AddFile(const fs::path &path) {
  const auto &extension = path.extension().string();

  if (!m_extension_category.contains(extension))
    return;

  const auto &category = m_extension_category[extension];
  std::lock_guard<std::mutex> lock(m_files_mutex);
  m_category_files[category].push_back(path.filename());
}

void DirectoryWalker::SubmitTask(const fs::path &path) {
  m_task_counter++;
  m_pool.submit_task([this, path]() {
    RecursiveFileSearch(path);
    if (--m_task_counter == 0)
      wait_cv.notify_one();
  });
}

void DirectoryWalker::RecursiveFileSearch(const fs::path &path) {
  try {
    for (const auto &dir_entry : fs::directory_iterator(path)) {

      if (dir_entry.is_directory()) {
        if (fs::is_symlink(dir_entry.path())) {
          continue;
        }

        std::string dir_name = dir_entry.path().filename().string();
        if (skip.contains(dir_name))
          continue;

        if (!dir_name.empty() && dir_name[0] != '.' && dir_name != "." &&
            dir_name != "..") {
          SubmitTask(dir_entry.path());
        }
      }

      if (dir_entry.is_regular_file())
        AddFile(dir_entry.path());
    }
  } catch (...) {
    return;
  }
}

std::unordered_map<std::string, std::vector<std::string>>
DirectoryWalker::GetFilenamesDevidedByCategory() {
  m_category_files.clear();

  SubmitTask(fs::path(m_path));

  std::unique_lock lock(m_wait_mutex);
  wait_cv.wait(lock, [this]() { return m_task_counter == 0; });

  return m_category_files;
}