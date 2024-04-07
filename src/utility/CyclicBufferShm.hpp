#pragma once

#include <atomic>
#include <cstddef>
#include <optional>
#include <string_view>

#include "utility/SharedMemoryBuff.hpp"

struct Datagram {
  enum { INSERT, READ, DELETE, READ_EMPTY } type;
  std::size_t id;
  std::string key;
  std::string value;
};

using Command = Datagram;
using Response = Datagram;

class CyclicBufferShm {
public:
  CyclicBufferShm(char *buffer, std::size_t buffer_size)
      : buffer_(buffer), buffer_size_(buffer_size) {}

  std::optional<Response> parseResponseFromBegin();

  std::optional<Command> parseNextCommandFromBegin();

  void writeReadEmptyResponse(std::size_t req_id, std::string_view key);
  void writeReadResponse(std::size_t req_id, std::string_view key,
                         std::string_view value);
  void writeInsert(std::size_t req_id, std::string_view key,
                   std::string_view value);
  void writeErase(std::size_t req_id, std::string_view key);
  void writeRead(std::size_t req_id, std::string_view key);

  void writeData(std::string_view data);

  void eraseBeginData(std::size_t sz) noexcept;
  void eraseLastData(std::size_t sz) noexcept;

  std::size_t usedSpace() const noexcept;
  std::size_t freeSpace() const noexcept;

private:
  void incBegin(int incVal = 1);

  std::size_t readInt();

  std::string readString(std::size_t sz);

  Command parseInsert();
  Command parseRead();
  Command parseDelete();

  Response parseReadResponse();
  Response parseReadEmpty();

  void incEnd(std::size_t val = 1);
  void decEnd(std::size_t val = 1);

  char *buffer_;
  std::size_t buffer_size_;

  std::size_t begin_{};
  std::size_t end_{};

  std::atomic_size_t used_space_{};
};