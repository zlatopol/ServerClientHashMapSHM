#include "HashMap.hpp"

#include <mutex>

void HashMap::erase(std::string_view key, std::size_t op_cnt) {
  const auto keyHash = calcHash(key);
  const auto bucketNum = bucket(keyHash);
  std::pair<KeyType, ValueType> insertKV{key,
                                         Value{std::nullopt, keyHash, op_cnt}};

  {
    std::unique_lock lock{mutexes_[bucketNum]};
    hashTable_[bucketNum].emplace_front(std::move(insertKV));
  }
}

void HashMap::put(std::string_view key, std::string_view value,
                  std::size_t op_cnt) {
  const auto keyHash = calcHash(key);
  const auto bucketNum = bucket(keyHash);

  std::pair<KeyType, ValueType> insertKV{
      key, Value{std::string{value}, keyHash, op_cnt}};

  {
    std::unique_lock lock{mutexes_[bucketNum]};
    hashTable_[bucketNum].emplace_front(std::move(insertKV));
  }
}

std::optional<std::string_view> HashMap::read(std::string_view key) const {
  ListType::const_iterator it{};
  ListType::const_iterator end{};
  const auto keyHash = calcHash(key);
  const auto bucketNum = bucket(keyHash);
  {
    std::shared_lock lock{mutexes_[bucketNum]};
    it = hashTable_.at(bucketNum).begin();
    end = hashTable_.at(bucketNum).end();
  }

  std::optional<std::string_view> data{};
  std::size_t op_cnt{};

  for (; it != end; ++it) {
    if (it->second.hash == keyHash && it->first == key &&
        op_cnt < it->second.counter) {
      data = it->second.data;
      op_cnt = it->second.counter;
    }
  }

  return data;
}

std::size_t HashMap::calcHash(std::string_view key) const {
  return std::hash<std::string_view>{}(key);
}

auto HashMap::bucket(std::size_t hash) const -> BucketCnt {
  return hash % hashTable_.size();
}
