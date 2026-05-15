#pragma once

#include <nlohmann/json.hpp>
#include <thread>

class Server {
public:
  Server(int port);
  void SetJson(const nlohmann::json &json);
  void Start();

  ~Server() {
    if (m_srv_thread.joinable())
      m_srv_thread.join();
  }

private:
  nlohmann::json m_json;
  const int m_port;
  std::mutex m_json_mutex;
  std::thread m_srv_thread;
};