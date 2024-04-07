#include "Client.hpp"

#include <cstring>

Client::Client(char *shared_memory)
    : shared_memory_(shared_memory), conn_semaphore_req_(connSemaphoreReq()),
      conn_semaphore_resp_(connSemaphoreResp()),
      conn_semaphore_rcv_(connSemaphoreRcv()) {}

Client::~Client() { running_ = false; }

std::optional<std::shared_ptr<ClientConnection>> Client::connect() {
  sem_post(conn_semaphore_req_);
  sem_wait(conn_semaphore_resp_);
  std::size_t offset{};
  std::memcpy(&offset, shared_memory_, sizeof(std::size_t));
  sem_post(conn_semaphore_rcv_);

  if (offset == -1) {
    return std::nullopt;
  } else {
    auto res = std::make_shared<ClientConnection>(
        offset, *(SharedMemoryBuff *)(shared_memory_ + offset));

    std::clog << "Established connection with offset " << res->buffOffset()
              << std::endl;
    std::thread{[this, res]() mutable {
      while (running_) {
        res->waitPing();
      }
    }}.detach();

    return res;
  }
}
