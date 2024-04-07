#include "Server.hpp"

#include <cstring>

#include "utility/SharedMemoryBuff.hpp"

void Server::runServerConnection(std::shared_ptr<ServerConnection> conn) {
  conn->handleCommand();
  threadPool_.submitTask([conn = std::move(conn), this]() mutable {
    runServerConnection(std::move(conn));
  });
}

void Server::pingServerConnections() {
  for (auto it = serverConnections_.begin(); it != serverConnections_.end();) {
    if (!(*it)->ping()) {
      std::clog << "Client with offset " << (*it)->buffOffset()
                << " is not responding" << std::endl;
      free_offsets_.push_back((*it)->buffOffset());
      (*it)->unlinkSemaphores();
      it = serverConnections_.erase(it);
    } else {
      ++it;
    }
  }
}

void Server::run() {
  while (true) {
//    pingServerConnections();
    const auto conn = waitServerConnection();
    if (conn.has_value()) {
      std::clog << "Established connection with offset "
                << (*conn)->buffOffset() << std::endl;

      serverConnections_.push_back(*conn);
      threadPool_.submitTask([conn = *conn, this]() mutable {
        runServerConnection(std::move(conn));
      });
    }
  }
}

void Server::writeOffset(std::size_t off) {
  std::memcpy(shared_memory_, &off, sizeof(off));
}

void Server::fillOffsets() {
  for (std::size_t offset = sizeof(std::size_t); offset < shared_memory_size_;
       offset += sizeof(SharedMemoryBuff)) {
    free_offsets_.push_back(offset);
  }
}

std::shared_ptr<ServerConnection>
Server::initServerConnectionInShm(std::size_t off) {
  SharedMemoryBuff *buff = new (shared_memory_ + off) SharedMemoryBuff{};

  return std::make_shared<ServerConnection>(off, *buff, hashMap_);
}

std::optional<std::shared_ptr<ServerConnection>>
Server::waitServerConnection() {
  sem_wait(conn_semaphore_req_);
  std::cout << "Got conn request" << std::endl;
  if (free_offsets_.empty()) {
    writeOffset(-1);
    sem_post(conn_semaphore_resp_);
    return std::nullopt;
  }
  const auto offset = free_offsets_.back();
  free_offsets_.pop_back();
  const auto serverConnection = initServerConnectionInShm(offset);
  writeOffset(offset);
  sem_post(conn_semaphore_resp_);
  sem_wait(conn_semaphore_rcv_);
  return serverConnection;
}
