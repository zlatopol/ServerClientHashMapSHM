#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string_view>
#include <thread>

#include "utility/ClientConnection.hpp"
#include "utility/ThreadPool.hpp"
#include "utility/constants.hpp"

#include "Client.hpp"
#include "utility/CyclicBufferShm.hpp"

int initShm() {
  if (int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_RDWR, 0777); shm != -1) {
    return shm;
  } else {
    perror("shm_open");
    return -1;
  }
}

std::string readStr(std::string line) {
  std::cout << line;
  std::string data;
  std::getline(std::cin, data);
  return data;
}

std::string random_string(size_t length) {
  auto randchar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

void testInsertPerformance(ClientConnection &client, int insertCounts, int id) {
  std::vector<std::future<void>> futures;
  for (int i = 0; i < insertCounts; ++i) {
    futures.push_back(client.insert(random_string(5), random_string(5)));
  }
  for (auto &f : futures) {
    f.get();
  }

  std::cout << std::to_string(id) + ": " + std::to_string(insertCounts) +
                   " inserts\n";
}

void testReadPerformance(ClientConnection &client, int readCounts, int id) {
  std::vector<std::future<std::optional<std::string>>> futures;
  for (int i = 0; i < readCounts; ++i) {
    futures.push_back(client.read(random_string(5)));
  }
  for (auto &f : futures) {
    f.get();
  }
  std::cout << std::to_string(id) + ": " + std::to_string(readCounts) +
                   " reads\n";
}

void testDeletePerformance(ClientConnection &client, int insertCounts, int id) {
  std::vector<std::future<void>> futures;
  for (int i = 0; i < insertCounts; ++i) {
    futures.push_back(client.erase(random_string(5)));
  }
  for (auto &f : futures) {
    f.get();
  }
  std::cout << std::to_string(id) + ": " + std::to_string(insertCounts) +
                   " deletes\n";
}

template <class F> void measureTime(F &&f, int id) {
  auto t1 = std::chrono::high_resolution_clock::now();
  f();
  auto t2 = std::chrono::high_resolution_clock::now();

  /* Getting number of milliseconds as a double. */
  std::chrono::duration<double, std::milli> ms_double = t2 - t1;

  std::cout << std::to_string(id) + ": " + std::to_string(ms_double.count()) +
                   "ms\n";
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "usage: " << argv[0] << " (clients_count) (operations_count)"
              << std::endl;
    return -1;
  }

  const auto clients_count = std::stoi(argv[1]);
  const auto operations_count = std::stoi(argv[2]);

  if (auto shm = initShm(); shm != -1) {
    char *addr = (char *)mmap(0, SHARED_MEMORY_OBJECT_SIZE + 1,
                              PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);

    Client client{addr};

    const auto operationsNumber = operations_count / clients_count;

    std::vector<std::shared_ptr<ClientConnection>> connections(clients_count);
    std::vector<std::thread> threads;
    threads.reserve(clients_count);

    for (int i = 0; i < clients_count; ++i) {
      auto conn = client.connect().value();
      connections[i] = conn;
    }
    measureTime(
        [&] {
          for (int i = 0; i < clients_count; ++i) {
            const auto &conn = connections[i];
            threads.emplace_back([conn, operationsNumber, id = i] {
              measureTime(
                  [conn, operationsNumber, id] {
                    testInsertPerformance(*conn, operationsNumber, id);
                  },
                  id);
              measureTime(
                  [conn, operationsNumber, id] {
                    testDeletePerformance(*conn, operationsNumber, id);
                  },
                  id);
              measureTime(
                  [conn, operationsNumber, id] {
                    testReadPerformance(*conn, operationsNumber, id);
                  },
                  id);
            });
          }
          for (auto &t : threads) {
            t.join();
          }
        },
        -1);
  } else {
    exit(EXIT_FAILURE);
  }
}
