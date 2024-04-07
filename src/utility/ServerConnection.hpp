#pragma once

#include "Connection.hpp"

class ServerConnection : public Connection {
public:
  ServerConnection(std::size_t offset, SharedMemoryBuff &shm_buffer,
                   HashMap &hashMap);

  bool handleCommand();

private:
  std::mutex output_mutex_;

  HashMap &hashMap_;
};