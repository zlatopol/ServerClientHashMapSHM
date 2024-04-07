#pragma once

#include <chrono>
#include <memory>
#include <semaphore.h>
#include <string_view>

#include "utility/HashMap.hpp"
#include "utility/ServerConnection.hpp"
#include "utility/ThreadPool.hpp"
#include "utility/constants.hpp"

class Server {
public:
  Server(char *shared_memory, std::size_t shared_memory_size,
         std::size_t hashMapSize)
      : shared_memory_(shared_memory), shared_memory_size_(shared_memory_size),
        conn_semaphore_req_(connSemaphoreReq(O_EXCL)),
        conn_semaphore_resp_(connSemaphoreResp(O_EXCL)),
        conn_semaphore_rcv_(connSemaphoreRcv(O_EXCL)), hashMap_(hashMapSize) {
    fillOffsets();
  }

  void run();

private:
  void runServerConnection(std::shared_ptr<ServerConnection> conn);
  void pingServerConnections();

  void fillOffsets();

  std::optional<std::shared_ptr<ServerConnection>> waitServerConnection();
  void writeOffset(std::size_t off);

  std::shared_ptr<ServerConnection> initServerConnectionInShm(std::size_t off);

  char *shared_memory_;
  std::size_t shared_memory_size_;

  sem_t *conn_semaphore_req_;
  sem_t *conn_semaphore_resp_;
  sem_t *conn_semaphore_rcv_;

  std::list<std::shared_ptr<ServerConnection>> serverConnections_;
  std::vector<std::size_t> free_offsets_;

  HashMap hashMap_;
  ThreadPool threadPool_;
};
