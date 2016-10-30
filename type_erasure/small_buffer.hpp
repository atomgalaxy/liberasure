#ifndef A9_SMALL_BUFFER_HPP
#define A9_SMALL_BUFFER_HPP

#include <cstdint>
#include <array>
#include <cassert>

namespace ubuf {

template <typename T>
using owner = T;

struct buffer_t {
  char* data;
  std::size_t size;
};

struct buffer_spec {
  std::size_t size;
  std::size_t align;
};

template <typename T>
auto allocate() -> buffer_t {
  using storage_type = std::aligned_storage_t<sizeof(T), alignof(T)>;
  return {reinterpret_cast<char*>(new storage_type()), sizeof(storage_type)};
}

/**
 * Get the next multiple of alignment.
 */
template <typename T>
auto align(char* buf_start) -> char* {
  auto start = reinterpret_cast<uintptr_t>(buf_start);
  auto misalignment = start % alignof(T);
  if (misalignment == 0) {
    return reinterpret_cast<char*>(start);
  } else {
    return reinterpret_cast<char*>(start + (alignof(T) - misalignment));
  }
}

template <std::size_t Size>
struct small_buffer {
  using buffer_type = std::array<char, Size>;

  small_buffer() : ptr{nullptr} {}
  ~small_buffer() { reset(); }

  small_buffer(small_buffer const&) = delete; // noncopyable
  small_buffer& operator=(small_buffer const&) = delete; // not copy assignable
  small_buffer(small_buffer&& x) = delete; // nonmovable
  small_buffer& operator=(small_buffer&& x) = delete; // not move assignable

  void reset() {
    if (!empty() && !is_internal()) {
      delete ptr;
    }
    ptr = nullptr;
  }

  char* get() { return ptr; }
  char const* get() const { return ptr; }

  auto empty() const -> bool { return ptr == nullptr; }

  template <typename U>
  auto allocate() -> buffer_t {
    assert(empty());
    auto const aligned_start = align<U>(buf_start());
    auto const aligned_end = aligned_start + sizeof(U);
    if (aligned_end <= buf_end()) { // fits inside buffer_
      ptr = aligned_start;
    } else {
      auto buf = ubuf::allocate<U>();
      ptr = buf.data;
    }
    return {ptr, sizeof(U)};
  };

  auto is_internal() const -> bool {
    assert(!empty());
    auto const cptr = reinterpret_cast<char*>(ptr);
    return buf_start() <= cptr && cptr < buf_end();
  }

  operator bool () const { return !empty(); }

  char& operator*() { return *ptr; }
  char const& operator*() const { return *ptr; }
  char* operator->() { return ptr; }
  char const* operator->() const { return ptr; }

  friend bool swap_if_not_internal(small_buffer& x, small_buffer& y) {
    using std::swap;
    if (x.is_internal() || y.is_internal()) { return false; }
    swap(x.ptr, y.ptr);
    return true;
  }

private:
  auto buf_start() -> char* { return buffer_.data(); }
  auto buf_end() -> char* { return buffer_.data() + buffer_.size(); }
  auto buf_start() const -> char const* { return buffer_.data(); }
  auto buf_end() const -> char const* { return buffer_.data() + buffer_.size(); }

  owner<char*> ptr;
  buffer_type buffer_;
};

template <>
struct small_buffer<0> {
  using buffer_type = std::array<char, 0>;

  small_buffer() : ptr{nullptr} {}

  small_buffer(small_buffer const&) = delete; // noncopyable
  small_buffer& operator=(small_buffer const&) = delete; // not copy assignable
  small_buffer(small_buffer&& x) = delete; // nonmovable
  small_buffer& operator=(small_buffer&& x) = delete; // not move assignable

  ~small_buffer() {
    reset();
  }

  void reset() {
    if (!empty()) {
      delete ptr;
    }
    ptr = nullptr;
  }

  char* get() { return ptr; }
  char const* get() const { return ptr; }

  auto empty() const -> bool { return ptr == nullptr; }

  template <typename U>
  auto allocate() -> buffer_t {
    auto buf = ubuf::allocate<U>();
    ptr = buf.data;
    return buf;
  }

  auto is_internal() const -> bool {
    assert(!empty());
    return false;
  }

  friend bool swap_if_not_internal(small_buffer& x, small_buffer& y) {
    using std::swap;
    swap(x.ptr, y.ptr);
    return true;
  }

  operator bool () const { return !empty(); }

  char& operator*() { return *ptr; }
  char const& operator*() const { return *ptr; }
  char* operator->() { return ptr; }
  char const* operator->() const { return ptr; }

private:
  owner<char*> ptr;
};

}

#endif
