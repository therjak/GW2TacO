#pragma once

#include <atomic>
#include <optional>

// A node in our lock-free queue
template <typename T>
struct Node {
  T data;
  std::atomic<Node<T>*> next;

  explicit Node(const T& val) : data(val), next(nullptr) {}
};

// Simple SPSC Lock-Free Queue
template <typename T>
class LockFreeQueue {
 private:
  // head is owned by the consumer, tail by the producer
  Node<T>* head;
  std::atomic<Node<T>*> tail;

 public:
  LockFreeQueue() {
    Node<T>* dummy = new Node<T>(T{});
    head = dummy;
    tail.store(dummy);
  }

  ~LockFreeQueue() {
    while (Node<T>* current = head) {
      head = current->next.load();
      delete current;
    }
  }

  void push(const T& value) {
    Node<T>* new_node = new Node<T>(value);
    Node<T>* old_tail = tail.load();
    old_tail->next.store(new_node);
    tail.store(new_node);
  }

  std::optional<T> pop() {
    Node<T>* next_node = head->next.load();

    if (next_node == nullptr) {
      return std::nullopt;  // Queue is empty
    }

    T result = next_node->data;

    Node<T>* old_head = head;
    head = next_node;
    delete old_head;

    return result;
  }
};
