#pragma once

#include "Connection.hpp"

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <unordered_map>

class ClientConnection : public Connection {
public:
  using Connection::Connection;

  std::future<void> insert(std::string_view key, std::string_view value);
  std::future<void> erase(std::string_view key);
  std::future<std::optional<std::string>> read(std::string_view key);

private:
  void readResponses();

  std::size_t reqId_{};
  std::atomic_bool running_{true};
  std::thread thread_{[this] { readResponses(); }};

  std::unordered_map<std::size_t, std::function<void(const Response &)>>
      complete_promises_func_;

  std::mutex mutex_;
};