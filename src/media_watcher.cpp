#include "media_watcher.hpp"

#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

MediaWatcher::MediaWatcher(
    const std::string &path,
    const std::unordered_map<std::string, std::string> &extension_category,
    long long interval, bool server, bool file_saving)
    : m_dw{path, extension_category}, m_srv{1234}, m_interval{interval},
      m_path{path}, m_run_server{server}, m_file_saving{file_saving} {}

void MediaWatcher::Iteraion() {
  std::cout << "Searching started\n";

  m_cached_json.clear();
  m_cached_json = m_dw.GetFilenamesDevidedByCategory();

  if (m_run_server) {
    m_srv.SetJson(m_cached_json);
  }

  if (m_file_saving) {
    fs::path full_path = fs::absolute("result.json");
    std::ofstream file{full_path};

    file << m_cached_json.dump(4);
    file.close();

    std::cout << "File saved at " << full_path << "\n";
  }

  std::cout << "Searching ended\n";
}

void MediaWatcher::Start() {
  if (m_run_server)
    m_srv.Start();

  while (1) {
    Iteraion();
    std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
  }
}