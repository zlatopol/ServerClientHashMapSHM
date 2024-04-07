#pragma once

#include <fcntl.h>
#include <semaphore.h>

#include <thread>

#include "SharedMemoryBuff.hpp"

const auto SEM_PERMS = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

const auto SHARED_MEMORY_OBJECT_NAME = "ShmHashMap";
const auto SHM_MAX_CONNECTIONS = std::thread::hardware_concurrency();
const auto SHARED_MEMORY_OBJECT_SIZE =
    sizeof(std::size_t) + sizeof(SharedMemoryBuff) * SHM_MAX_CONNECTIONS;

inline sem_t *connSemaphoreReq(int flags = 0) {
  if (auto sem =
          sem_open("connSemaphoreReq", O_RDWR | O_CREAT | flags, SEM_PERMS, 0);
      sem != SEM_FAILED) {
    return sem;
  }
  perror("cannot open conn req sem");
  std::exit(-1);
}

inline sem_t *connSemaphoreResp(int flags = 0) {
  if (auto sem =
          sem_open("connSemaphoreResp", O_RDWR | O_CREAT | flags, SEM_PERMS, 0);
      sem != SEM_FAILED) {
    return sem;
  }
  perror("cannot open conn resp sem");
  std::exit(-1);
}

inline sem_t *connSemaphoreRcv(int flags = 0) {
  if (auto sem =
          sem_open("connSemaphoreRcv", O_RDWR | O_CREAT | flags, SEM_PERMS, 0);
      sem != SEM_FAILED) {
    return sem;
  }
  perror("cannot open conn rcv sem");
  std::exit(-1);
}
