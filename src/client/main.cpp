#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string_view>

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

int main() {
  if (auto shm = initShm(); shm != -1) {
    char *addr = (char *)mmap(0, SHARED_MEMORY_OBJECT_SIZE + 1,
                              PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);

    Client client{addr};
    auto conn = client.connect().value();

    while (true) {
      try {
        const std::string command =
            readStr("Enter operation: read|insert|delete: ");

        if (command == "read") {
          const auto key = readStr("Enter key: ");
          std::cout << "Read value: "
                    << conn->read(key).get().value_or("not found") << std::endl;
        } else if (command == "insert") {
          const auto key = readStr("Enter key: ");
          const auto val = readStr("Enter value: ");
          conn->insert(key, val);
        } else if (command == "delete") {
          const auto key = readStr("Enter key: ");
          conn->erase(key);
        } else {
          std::cerr << "invalid operation: " << command << std::endl;
        }
      } catch (const std::exception &e) {
        std::cerr << "error in operation: " << e.what() << std::endl;
      }
    }
  } else {
    exit(EXIT_FAILURE);
  }
}