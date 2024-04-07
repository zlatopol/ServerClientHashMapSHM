#pragma once

#include <chrono>
#include <list>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class HashMap {
public:
  HashMap(std::size_t size) : hashTable_(size), mutexes_(size) {}

  void erase(std::string_view key, std::size_t op_cnt);
  void put(std::string_view key, std::string_view value, std::size_t op_cnt);
  std::optional<std::string_view> read(std::string_view key) const;

private:
  struct Value {
    std::optional<std::string> data;
    std::size_t hash;
    std::size_t counter;
  };

  using ValueType = Value;

  using KeyType = std::string;
  using ListType = std::list<std::pair<KeyType, ValueType>>;
  using ContainerType = std::vector<ListType>;
  using BucketCnt = ContainerType::size_type;

  std::size_t calcHash(std::string_view key) const;

  BucketCnt bucket(std::size_t hash) const;

  ContainerType hashTable_;
  mutable std::vector<std::shared_mutex> mutexes_;
};
