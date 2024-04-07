#include "Connection.hpp"

#include <cassert>
#include <iostream>
#include <thread>
#include <cstring>

#include "utility/constants.hpp"

Connection::Connection(std::size_t offset, SharedMemoryBuff &shm_buffer)
    : offset_(offset), shm_buffer_(shm_buffer),
      requests_shm_(shm_buffer.requests_list.data(),
                    shm_buffer.requests_list.size()),
      responses_shm_(shm_buffer.reponses_list.data(),
                     shm_buffer.reponses_list.size()) {
  openSemaphores();
}

void Connection::openSemaphores() {
  if ((sem_req_ = sem_open(semReqNameFromOffset(offset_).c_str(),
                           O_RDWR | O_CREAT, SEM_PERMS, 0)) == SEM_FAILED) {
    throw std::runtime_error{std::strerror(errno)};
  }

  if ((sem_resp_ = sem_open(semRespNameFromOffset(offset_).c_str(),
                            O_RDWR | O_CREAT, SEM_PERMS, 0)) == SEM_FAILED) {
    throw std::runtime_error{std::strerror(errno)};
  }

  if ((sem_ping_req_ = sem_open(semPingNameFromOffset(offset_).c_str(),
                                O_RDWR | O_CREAT, SEM_PERMS, 0)) ==
      SEM_FAILED) {
    throw std::runtime_error{std::strerror(errno)};
  }

  if ((sem_ping_resp_ = sem_open(semPingRespNameFromOffset(offset_).c_str(),
                                 O_RDWR | O_CREAT, SEM_PERMS, 0)) ==
      SEM_FAILED) {
    throw std::runtime_error{std::strerror(errno)};
  }
}

void Connection::unlinkSemaphores() {
  sem_unlink(semReqNameFromOffset(offset_).c_str());
  sem_unlink(semPingNameFromOffset(offset_).c_str());
  sem_unlink(semRespNameFromOffset(offset_).c_str());
  sem_unlink(semPingRespNameFromOffset(offset_).c_str());
}

void Connection::insert(std::size_t reqId, std::string_view key,
                        std::string_view value) {
  requests_shm_.writeInsert(reqId, key, value);
  sem_post(sem_req_);
}

void Connection::erase(std::size_t reqId, std::string_view key) {
  requests_shm_.writeErase(reqId, key);
  sem_post(sem_req_);
}

void Connection::read(std::size_t reqId, std::string_view key) {
  requests_shm_.writeRead(reqId, key);
  sem_post(sem_req_);
}

std::size_t Connection::buffOffset() { return offset_; }

bool Connection::ping() {
  sem_post(sem_ping_req_);
  errno = 0;
  for (int i = 0; i < 5; ++i) {
    sem_trywait(sem_ping_resp_);
    if (errno != EAGAIN) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return false;
}

void Connection::waitPing() {
  sem_wait(sem_ping_req_);
  sem_post(sem_ping_resp_);
}

auto Connection::parseNextCommand() -> std::optional<Command> {
  if (sem_trywait(sem_req_) != 0)
    return std::nullopt;

  return requests_shm_.parseNextCommandFromBegin();
}
