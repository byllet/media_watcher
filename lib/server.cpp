#include "server.hpp"
#include "httplib.h"
#include <mutex>
#include <thread>

Server::Server(int port) : m_json{}, m_port{port} {}

void Server::SetJson(const nlohmann::json &json) {
  std::lock_guard lock(m_json_mutex);
  m_json = json;
}

void Server::Start() {
  m_srv_thread = std::thread([&]() {
    httplib::Server server;
    server.Get("/media_files",
               [&](const httplib::Request &req, httplib::Response &res) {
                 std::cout << "[Server] got GET request\n";
                 std::lock_guard lock(m_json_mutex);
                 res.set_content(m_json.dump(4), "application/json");
                 std::cout << "[Server] request handled\n";
               });

    std::cout << "[Server] listen at http://localhost:" << m_port
              << "/media_files\n";
    server.listen("localhost", m_port);
  });
}