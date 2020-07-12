/*
 * Copyright 2015, 2016 Gašper Ažman
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>
#include <array>
#include <cassert>

namespace erasure {

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
  auto operator=(small_buffer const&) -> small_buffer& = delete; // not copy assignable
  small_buffer(small_buffer&& x) = delete; // nonmovable
  auto operator=(small_buffer&& x) -> small_buffer& = delete; // not move assignable

  void reset() {
    if (!empty() && !is_internal()) {
      delete ptr;
    }
    ptr = nullptr;
  }

  auto get() -> char* { return ptr; }
  auto get() const -> char const* { return ptr; }

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

  auto operator*() -> char& { return *ptr; }
  auto operator*() const -> char const& { return *ptr; }
  auto operator->() -> char* { return ptr; }
  auto operator->() const -> char const* { return ptr; }

  friend auto swap_if_not_internal(small_buffer& x, small_buffer& y) -> bool {
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
  auto operator=(small_buffer const&) -> small_buffer& = delete; // not copy assignable
  small_buffer(small_buffer&& x) = delete; // nonmovable
  auto operator=(small_buffer&& x) -> small_buffer& = delete; // not move assignable

  ~small_buffer() {
    reset();
  }

  void reset() {
    if (!empty()) {
      delete ptr;
    }
    ptr = nullptr;
  }

  auto get() -> char* { return ptr; }
  auto get() const -> char const* { return ptr; }

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

  friend auto swap_if_not_internal(small_buffer& x, small_buffer& y) -> bool {
    using std::swap;
    swap(x.ptr, y.ptr);
    return true;
  }

  operator bool () const { return !empty(); }

  auto operator*() -> char& { return *ptr; }
  auto operator*() const -> char const& { return *ptr; }
  auto operator->() -> char* { return ptr; }
  auto operator->() const -> char const* { return ptr; }

private:
  owner<char*> ptr;
};

}

}
