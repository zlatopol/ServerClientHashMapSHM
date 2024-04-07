#include "ServerConnection.hpp"

ServerConnection::ServerConnection(std::size_t offset,
                                   SharedMemoryBuff &shm_buffer,
                                   HashMap &hashMap)
    : Connection(offset, shm_buffer), hashMap_(hashMap) {
  unlinkSemaphores();
  openSemaphores();
}

bool ServerConnection::handleCommand() {
  auto nextCommand = parseNextCommand();
  if (!nextCommand.has_value()) {
    return false;
  }

  switch (nextCommand->type) {
  case Command::INSERT: {
    const auto op_cnt = static_cast<std::size_t>(time(NULL));
    hashMap_.put(nextCommand->key, nextCommand->value, op_cnt);
    {
      std::unique_lock lock{output_mutex_};
      responses_shm_.writeInsert(nextCommand->id, nextCommand->key,
                                 nextCommand->value);
      responses_shm_.parseResponseFromBegin();
    }
    sem_post(sem_resp_);
    break;
  }
  case Command::DELETE: {
    const auto op_cnt = static_cast<std::size_t>(time(NULL));
    hashMap_.erase(nextCommand->key, op_cnt);

    {
      std::unique_lock lock{output_mutex_};
      responses_shm_.writeErase(nextCommand->id, nextCommand->key);
      responses_shm_.parseResponseFromBegin();
    }
    sem_post(sem_resp_);
    break;
  }
  case Command::READ: {
    const auto value = hashMap_.read(nextCommand->key);

    {
      std::unique_lock lock{output_mutex_};

      if (value.has_value()) {
        responses_shm_.writeReadResponse(nextCommand->id, nextCommand->key,
                                         *value);
      } else {
        responses_shm_.writeReadEmptyResponse(nextCommand->id,
                                              nextCommand->key);
      }
      responses_shm_.parseResponseFromBegin();
    }

    sem_post(sem_resp_);
    break;
  }
  default:
    break;
  }
  return true;
}
