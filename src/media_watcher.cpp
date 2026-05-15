#include "media_watcher.hpp"

#include "httplib.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <unordered_set>

const std::unordered_set<std::string> skip = {"Library"};

void MediaWatcher::AddFile(const fs::path &path) {
  const auto &extension = path.extension().string();

  if (!m_extension_category.contains(extension))
    return;

  const auto &category = m_extension_category[extension];
  std::lock_guard<std::mutex> lock(m_files_mutex);
  category_files[category].push_back(path.filename());
}

void MediaWatcher::SubmitTask(const fs::path &path) {
  m_task_counter++;
  m_pool.submit_task([this, path]() {
    RecursiveFileSearch(path);
    if (--m_task_counter == 0)
      wait_cv.notify_one();
  });
}

void MediaWatcher::RecursiveFileSearch(const fs::path &path) {
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

void MediaWatcher::Iteraion() {
  {
    std::lock_guard lock(m_files_mutex);
    category_files.clear();
  }
  std::cout << "Started\n";

  {
    SubmitTask(fs::path(m_path));
    std::unique_lock lock(m_wait_mutex);
    wait_cv.wait(lock, [this]() { return m_task_counter == 0; });
  }

  using json = nlohmann::json;
  m_cached_json.clear();
  m_cached_json = category_files;

  if (m_file_saving) {
    fs::path full_path = fs::absolute("result.json");
    std::ofstream file{full_path};

    file << m_cached_json.dump(4);
    file.close();
    std::cout << "File saved at " << full_path << "\n";
  }

  std::cout << "Ended\n";
}

void MediaWatcher::StartServer() {
  httplib::Server server;
  server.Get("/media_files",
             [&](const httplib::Request &req, httplib::Response &res) {
               std::cout << "[Server] got GET request\n";
               std::lock_guard<std::mutex> lock(m_files_mutex);
               nlohmann::json j = category_files;
               res.set_content(j.dump(4), "application/json");
               std::cout << "Request handled\n";
             });

  std::cout << "[Server] listen at http://localhost:1234/media_files\n";
  server.listen("localhost", 1234);
}

void MediaWatcher::Start() {

  std::thread server_thread([this]() {
    if (m_server)
      StartServer();
  });

  while (1) {
    Iteraion();
    std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
  }

  if (m_server)
    server_thread.join();
}