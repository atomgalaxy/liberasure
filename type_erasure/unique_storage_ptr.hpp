#pragma once

#include <cstddef>
#include <array>
#include <utility>
#include <new>
#include <cstdlib>

template <typename T>
using owner = T;

template <typename T, std::size_t Size>
struct unique_storage_ptr {
  owner<T*> ptr;
  std::array<char, Size> buffer;

  unique_storage_ptr() : ptr{nullptr} {}
  unique_storage_ptr(unique_storage_ptr&) = delete;
  unique_storage_ptr& operator=(unique_storage_ptr&) = delete;
  unique_storage_ptr(unique_storage_ptr&& x) : ptr{nullptr} {
    if (x.is_heap()) {
      std::swap(ptr, x.ptr);
    } else {

    }
  }
  ~unique_storage_ptr() { reset(); }

  T& operator*() { return *ptr; }
  T const& operator*() const { return *ptr; }

  bool empty() const { return ptr; }
  explicit operator bool() const { return ptr; }

  bool is_heap() const { return buffer.begin() <= ptr < buffer.end(); }
  void reset() {
    if (ptr) { ptr->~T(); }
    if (is_heap()) { free(ptr); }
    ptr = nullptr;
  }

  template <typename U, typename... Ts>
  friend T& create(unique_storage_ptr& x, Ts... xs) {
    assert(x.empty());
    auto where = (sizeof(U) <= Size) ? x.buffer.data : malloc(sizeof(U));
    x.ptr = new (where) U (std::forward<Ts>(xs)...);
  }
};

