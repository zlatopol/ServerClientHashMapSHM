#pragma once

#include <atomic>

template <class Value> class HashMapAsyncList {
  struct Node {
    std::optional<Value> value;
    Node *next;
  };

public:
  const Node *begin() const { return head_.load(); }
  const Node *end() const { return &fake_node_; }

  void pushFront(Value val) {
    Node *new_node = new Node{std::move(val), head_.load()};

    while (head_.compare_exchange_weak(new_node->next, new_node))
      ;
  }

  ~HashMapAsyncList() {
    Node *it = head_.load();
    while (it != &fake_node_) {
      Node *next = it->next;
      delete it;
      it = next;
    }
  }

private:
  Node fake_node_{std::nullopt, &fake_node_};
  std::atomic<Node *> head_{&fake_node_};
};