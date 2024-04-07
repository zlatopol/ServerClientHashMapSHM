#pragma once

#include <chrono>
#include <mutex>
#include <semaphore.h>
#include <stdexcept>
#include <string_view>

#include "utility/CyclicBufferShm.hpp"
#include "utility/HashMap.hpp"
#include "utility/SharedMemoryBuff.hpp"
#include "utility/ThreadPool.hpp"

class Connection {
public:
  Connection(std::size_t offset, SharedMemoryBuff &shm_buffer);

  void insert(std::size_t reqId, std::string_view key, std::string_view value);
  void erase(std::size_t reqId, std::string_view key);
  void read(std::size_t reqId, std::string_view key);

  bool ping();
  void waitPing();

  void openSemaphores();
  void unlinkSemaphores();

  std::size_t buffOffset();

protected:
  std::optional<Command> parseNextCommand();

  sem_t *sem_req_;
  sem_t *sem_resp_;
  sem_t *sem_ping_req_;
  sem_t *sem_ping_resp_;

  std::size_t offset_;
  SharedMemoryBuff &shm_buffer_;

  CyclicBufferShm requests_shm_;
  CyclicBufferShm responses_shm_;
};