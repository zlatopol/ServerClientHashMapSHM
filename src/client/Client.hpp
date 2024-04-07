#pragma once

#include <semaphore.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "utility/ClientConnection.hpp"
#include "utility/ThreadPool.hpp"
#include "utility/constants.hpp"

class Client {
public:
  Client(char *shared_memory);

  std::optional<std::shared_ptr<ClientConnection>> connect();

  ~Client();

private:
  char *shared_memory_;

  sem_t *conn_semaphore_req_;
  sem_t *conn_semaphore_resp_;
  sem_t *conn_semaphore_rcv_;

  std::atomic_bool running_{true};
};
