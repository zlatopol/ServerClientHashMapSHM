#include <iostream>

#include <atomic>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "Server.hpp"
#include "utility/constants.hpp"

int initShm() {
  if (int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_CREAT | O_RDWR, 0777);
      shm != -1) {
    if (ftruncate(shm, SHARED_MEMORY_OBJECT_SIZE + 1) == -1) {
      perror("ftruncate");
      shm_unlink(SHARED_MEMORY_OBJECT_NAME);
      return -1;
    }
    return shm;
  } else {
    perror("shm_open");
    return -1;
  }
}

void unlinkShm() { shm_unlink(SHARED_MEMORY_OBJECT_NAME); }
void unlinkSem() {
  sem_unlink("connSemaphoreReq");
  sem_unlink("connSemaphoreResp");
  sem_unlink("connSemaphoreRcv");
}
void atExit() {
  unlinkShm();
  unlinkSem();
}
void terminateHandle(int) {
  atExit();
  std::abort();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " (hashmap size)" << std::endl;
    exit(EXIT_FAILURE);
  }

  unsigned long hashMapSize{};
  try {
    hashMapSize = std::stoul(argv[1]);
  } catch (const std::exception &ex) {
    std::cerr << "first argument: " << ex.what() << std::endl;
    std::cout << "usage: " << argv[0] << " (hashmap size)" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::atexit(atExit);
  signal(SIGINT, terminateHandle);

  if (auto shm = initShm(); shm != -1) {
    char *addr = (char *)mmap(0, SHARED_MEMORY_OBJECT_SIZE + 1,
                              PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);

    Server server{addr, SHARED_MEMORY_OBJECT_SIZE, hashMapSize};
    server.run();
  } else {
    exit(EXIT_FAILURE);
  }
}
