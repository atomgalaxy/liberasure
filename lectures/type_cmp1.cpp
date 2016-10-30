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

#include <iostream>
#include <utility>
#include <cstdlib>

class str_const {
private:
  const char* const p_;
  const size_t sz_;

public:
  template <size_t N>
  constexpr str_const(const char(&a)[N])
      : p_(a)
      , sz_(N - 1) {}

  constexpr char operator[](size_t n) const {
    return n < sz_ ? p_[n] : throw std::out_of_range("");
  }

  constexpr size_t size() const { return sz_; }  // size()
};

template <char... letters>
struct string_t {
  static char const* c_str() {
    static constexpr char string[] = {letters..., '\0'};
    return string;
    }
};

template <str_const const& str, size_t... I>
auto constexpr expand(std::index_sequence<I...>) {
  return string_t<str[I]...>{};
}
template <str_const const& str>
using string_const_to_type =
decltype(expand<str>(std::make_index_sequence<str.size()>{}));

constexpr str_const hello{"Hello World"};
using hello_t = string_const_to_type<hello>;

constexpr str_const oy{[] { return __PRETTY_FUNCTION__ }()};
int main() {
//  char c = hello_t{};  // Compile error to print type

  using pfunc_t = string_const_to_type<oy>;
  std::cout << hello_t::c_str();
  return 0;
}
