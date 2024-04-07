#include "CyclicBufferShm.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>

std::size_t CyclicBufferShm::usedSpace() const noexcept {
  return used_space_.load(std::memory_order_relaxed);
}

// format: E(reqId):(keySize):(key)
void CyclicBufferShm::writeReadEmptyResponse(std::size_t req_id,
                                             std::string_view key) {
  const auto keyIdStr = std::to_string(req_id);
  const auto keySize = std::to_string(key.size());

  writeData("E");
  writeData(keyIdStr);
  writeData(":");
  writeData(keySize);
  writeData(":");
  writeData(key);
}

void CyclicBufferShm::incBegin(int incVal) {
  used_space_.fetch_sub(incVal, std::memory_order_relaxed);
  begin_ = (begin_ + incVal) % buffer_size_;
}

std::size_t CyclicBufferShm::readInt() {
  std::string intStr{};
  while ('0' <= buffer_[begin_] && buffer_[begin_] <= '9') {
    intStr += buffer_[begin_];
    incBegin();
  }

  if (intStr.empty()) {
    throw std::invalid_argument{"error in parsing int"};
  }

  return std::stoull(intStr);
}

std::string CyclicBufferShm::readString(std::size_t sz) {
  if (begin_ + sz < buffer_size_) {
    const auto res = std::string{buffer_ + begin_, sz};
    incBegin(sz);
    return res;
  } else {
    std::string res{};
    const auto svCursor = begin_;
    res += std::string{buffer_ + begin_, sz};
    incBegin(sz);
    if (svCursor < begin_) {
      throw std::out_of_range{"too large string"};
    }
    res += std::string{buffer_, begin_};
    return res;
  }
}

// format: I(reqId):(keySize):(valueSize):(keyString)(ValueString)
auto CyclicBufferShm::parseInsert() -> Command {
  assert(buffer_[begin_] == 'I');
  incBegin();

  const auto reqId = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto valueSize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);
  auto valueString = readString(valueSize);

  return Command{Command::INSERT, reqId, std::move(keyString),
                 std::move(valueString)};
}

// format: R(reqId):(keySize):(keyString)
auto CyclicBufferShm::parseRead() -> Command {
  assert(buffer_[begin_] == 'R');
  incBegin();

  const auto reqId = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);

  return Command{Command::READ, reqId, std::move(keyString), {}};
}

// format: D(reqId):(keySize):(keyString)
auto CyclicBufferShm::parseDelete() -> Command {
  assert(buffer_[begin_] == 'D');
  incBegin();

  const auto reqId = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);

  return Command{Command::DELETE, reqId, std::move(keyString), {}};
}

// format: R(reqId):(keySize):(valueSize):(keyString)(value)
auto CyclicBufferShm::parseReadResponse() -> Command {
  assert(buffer_[begin_] == 'R');
  incBegin();

  const auto reqId = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto valueSize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);
  auto valueString = readString(valueSize);

  return Command{Command::READ, reqId, std::move(keyString),
                 std::move(valueString)};
}

// format: E(reqId):(keySize):(key)
Response CyclicBufferShm::parseReadEmpty() {
  assert(buffer_[begin_] == 'E');
  incBegin();

  const auto reqId = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);

  return Response{Command::READ_EMPTY, reqId, std::move(keyString), {}};
}

std::optional<Response> CyclicBufferShm::parseResponseFromBegin() {
  switch (buffer_[begin_]) {
  case 'I':
    return parseInsert();
  case 'D':
    return parseDelete();
  case 'R':
    return parseReadResponse();
  case 'E':
    return parseReadEmpty();
  }
  throw std::runtime_error{std::string{"Unhandled Response type: "} +
                           buffer_[begin_]};
}

auto CyclicBufferShm::parseNextCommandFromBegin() -> std::optional<Command> {
  switch (buffer_[begin_]) {
  case 'I':
    return parseInsert();
  case 'D':
    return parseDelete();
  case 'R':
    return parseRead();
  }
  throw std::runtime_error{std::string{"Unhandled Command type: "} +
                           buffer_[begin_]};
}

// format: R(reqId):(keySize):(valueSize):(key)(value)
void CyclicBufferShm::writeReadResponse(std::size_t reqId, std::string_view key,
                                        std::string_view value) {
  const auto keyIdStr = std::to_string(reqId);
  const auto keySize = std::to_string(key.size());
  const auto valueSize = std::to_string(value.size());

  writeData("R");
  writeData(keyIdStr);
  writeData(":");
  writeData(keySize);
  writeData(":");
  writeData(valueSize);
  writeData(":");
  writeData(key);
  writeData(value);
}

// format: I(reqId):(keySize):(valueSize):(key)(value)
void CyclicBufferShm::writeInsert(std::size_t reqId, std::string_view key,
                                  std::string_view value) {
  const auto keyIdStr = std::to_string(reqId);
  const auto keySize = std::to_string(key.size());
  const auto valueSize = std::to_string(value.size());

  writeData("I");
  writeData(keyIdStr);
  writeData(":");
  writeData(keySize);
  writeData(":");
  writeData(valueSize);
  writeData(":");
  writeData(key);
  writeData(value);
}

// format: E(reqId):(keySize):(key)
void CyclicBufferShm::writeErase(std::size_t reqId, std::string_view key) {
  const auto keyIdStr = std::to_string(reqId);
  const auto keySize = std::to_string(key.size());

  writeData("D");
  writeData(keyIdStr);
  writeData(":");
  writeData(keySize);
  writeData(":");
  writeData(key);
}

// format: R(reqId):(keySize):(key)
void CyclicBufferShm::writeRead(std::size_t reqId, std::string_view key) {
  const auto keyIdStr = std::to_string(reqId);
  const auto keySize = std::to_string(key.size());

  writeData("R");
  writeData(keyIdStr);
  writeData(":");
  writeData(keySize);
  writeData(":");
  writeData(key);
}

std::size_t CyclicBufferShm::freeSpace() const noexcept {
  return buffer_size_ - usedSpace() - 1;
}

void CyclicBufferShm::incEnd(std::size_t val) {
  used_space_.fetch_add(val, std::memory_order_relaxed);

  end_ = (end_ + val) % buffer_size_;
}

void CyclicBufferShm::decEnd(std::size_t val) {
  used_space_.fetch_sub(val, std::memory_order_relaxed);

  if (end_ > val) {
    end_ -= val;
  } else {
    val -= end_;
    end_ = buffer_size_ - val;
  }
}

void CyclicBufferShm::eraseBeginData(std::size_t sz) noexcept {
  assert(sz < usedSpace());
  incBegin(sz);
}

void CyclicBufferShm::eraseLastData(std::size_t sz) noexcept {
  assert(sz < usedSpace());
  decEnd(sz);
}

void CyclicBufferShm::writeData(std::string_view data) {
  if (freeSpace() < data.size()) {
    throw std::out_of_range{"out of space in shared memory"};
  }

  if (end_ + data.size() <= buffer_size_) {
    std::memcpy(buffer_ + end_, data.data(), data.size());
    incEnd(data.size());
  } else {
    const auto sz = data.size();
    std::memcpy(buffer_ + end_, data.data(), buffer_size_ - end_);
    data.remove_prefix(buffer_size_ - end_);
    std::memcpy(buffer_, data.data(), data.size());
    incEnd(sz);
  }
}
