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

#include "demangle.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <vector>

namespace dbg_util {

enum class operation {
  DEFAULT_CONSTRUCTION,
  VALUE_CONSTRUCTION,
  COPY_CONSTRUCTION,
  MOVE_CONSTRUCTION,
  COPY_ASSIGNMENT,
  MOVE_ASSIGNMENT,
  DESTRUCTION,
  SWAP,
  EQUALS
};
auto operator<<(std::ostream &out, operation op) -> std::ostream & {
  switch (op) {
  case operation::DEFAULT_CONSTRUCTION:
    return out << "DEFAULT_CONSTRUCTION";
  case operation::VALUE_CONSTRUCTION:
    return out << "VALUE_CONSTRUCTION";
  case operation::COPY_CONSTRUCTION:
    return out << "COPY_CONSTRUCTION";
  case operation::MOVE_CONSTRUCTION:
    return out << "MOVE_CONSTRUCTION";
  case operation::COPY_ASSIGNMENT:
    return out << "COPY_ASSIGNMENT";
  case operation::MOVE_ASSIGNMENT:
    return out << "MOVE_ASSIGNMENT";
  case operation::DESTRUCTION:
    return out << "DESTRUCTION";
  case operation::SWAP:
    return out << "SWAP";
  case operation::EQUALS:
    return out << "EQUALS";
  }
  assert(false && "Unreachable");
}

template <typename T>
struct instrumented;

template <typename>
uint64_t current_id = 0;
inline auto get_id() -> uint64_t { return current_id<void> ++; }
inline void reset_numbering() { current_id<void> = 0; }

template <typename>
std::vector<std::tuple<uint64_t, uint64_t, operation>> trace_ = {};

inline auto trace() -> auto & { return trace_<void>; }
template <typename T, typename U>
inline void add_to_trace(instrumented<T> const &x, instrumented<U> const &y,
                         operation op) {
  trace().emplace_back(x.id, y.id, op);
}
template <typename T>
inline void add_to_trace(instrumented<T> const &x, operation op) {
  trace().emplace_back(x.id, -1, op);
}
inline void clear_trace() { trace().clear(); }
using trace_type = std::decay_t<decltype(trace())>;
template <typename... Tuples>
auto tuples_to_trace(Tuples const &... ts) {
  return trace_type{ts...};
}
template <typename... Tuples>
auto trace_is(Tuples const &... ts) -> bool {
  return trace() == tuples_to_trace(ts...);
}

void print_trace(std::ostream &o, trace_type const &t = trace()) {
  using std::get;
  for (auto const &line : t) {
    if (get<1>(line) == static_cast<decltype(get<1>(line))>(-1)) {
      o << "[trace]: " << get<2>(line) << " on " << get<0>(line) << "\n";
    } else {
      o << "[trace]: " << get<2>(line) << " between " << get<0>(line) << " and "
        << get<1>(line) << "\n";
    }
  }
}

namespace detail {
template <typename T, typename U>
constexpr auto swap_is_noexcept(T &x, U &y) -> bool {
  using std::swap;
  return noexcept(swap(x, y));
}

} // namespace detail

template <typename T>
struct instrumented {
  template <typename ValueType>
  explicit instrumented(ValueType &&value)
      : id(get_id()), value(std::forward<T>(value)) {
    add_to_trace(*this, operation::VALUE_CONSTRUCTION);
  }
  instrumented() : id(get_id()) {
    add_to_trace(*this, operation::DEFAULT_CONSTRUCTION);
  }
  instrumented(instrumented const &x) : id(get_id()), value(x.value) {
    add_to_trace(*this, x, operation::COPY_CONSTRUCTION);
  }
  instrumented(instrumented &&x) : id(get_id()), value(std::move(x.value)) {
    add_to_trace(*this, x, operation::MOVE_CONSTRUCTION);
  }
  auto operator=(instrumented const &x) -> instrumented & {
    add_to_trace(*this, x, operation::COPY_ASSIGNMENT);
    value = x.value;
    return *this;
  }
  auto operator=(instrumented &&x) -> instrumented & {
    add_to_trace(*this, x, operation::MOVE_ASSIGNMENT);
    value = std::move(x.value);
    return *this;
  }
  ~instrumented() {
    try {
      add_to_trace(*this, operation::DESTRUCTION);
    } catch (...) {
    } // don't die if cerr somehow can't be written to.
  }

  friend auto operator<<(std::ostream &o, instrumented const &x)
      -> std::ostream & {
    return o << demangle(typeid(x).name()) << "{" << x.id << ", " << x.value
             << "}";
  }

  friend void swap(instrumented &x, instrumented &y) noexcept(
      noexcept(detail::swap_is_noexcept(x.value, y.value))) {
    using std::swap;
    add_to_trace(x, y, operation::SWAP);
    swap(x.value, y.value);
  }

  friend auto operator==(instrumented const &x, instrumented const &y) -> bool {
    add_to_trace(x, y, operation::EQUALS);
    return x.value == y.value;
  }

  uint64_t id;
  T value;
};

template <typename T>
auto make_instrumented(T &&x) {
  return instrumented<std::decay_t<T>>{std::forward<T>(x)};
}

template <typename... Tuples>
void assert_trace_is_and_clear_(std::string const &file, int line,
                                Tuples const &... ts) {
  if (!dbg_util::trace_is(ts...)) {
    std::cerr << "Asstion assert_trace_is_and_clear() at " << file << ":"
              << line << " falied.\n"
              << "Trace:\n"
              << "^^^^^^\n";
    dbg_util::print_trace(std::cerr);
    std::cerr << "Expected trace:\n"
              << "^^^^^^^^^^^^^^^\n";
    dbg_util::print_trace(std::cerr, dbg_util::tuples_to_trace(ts...));
    exit(1);
  }
  dbg_util::clear_trace();
}
#define ASSERT_AND_CLEAR_TRACE_IS(...)                                         \
  ::dbg_util::assert_trace_is_and_clear_(__FILE__, __LINE__, __VA_ARGS__)

} // namespace dbg_util
