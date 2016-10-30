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

#include "type_erasure/meta.hpp"
#include <cstddef>
#include <utility>

namespace compile_time_string {
inline namespace {
template <size_t I, size_t N>
constexpr auto sget(const char (&arr)[N]) {
  return arr[I];
}

template <char C>
using char_ = std::integral_constant<char, C>;

template <int I>
using int_ = std::integral_constant<int, I>;

template <char... chars>
struct string_;

template <size_t N, typename str_type>
struct to_string_helper {
  template <size_t index>
  struct char_at_ {
      using type = char_<str_type{}.char_at(index)>;
  };
  template <typename Index>
  using char_at = typename char_at_<Index{}>::type;
  using type = meta::map<meta::index_sequence<N>, char_at>;
};

template <typename... Ts>
meta::typelist<Ts...> types(Ts...) { return {}; }
} // anon namespace
} // compile_time_string

#define STRING_C(literal)                                              \
  ([] {                                                                \
    using compile_time_string::to_string_helper;                       \
    struct constexpr_string_type {                                     \
      constexpr auto char_at(size_t i) { return (literal)[i]; }        \
    };                                                                 \
    return (typename to_string_helper<sizeof((literal)),               \
                                      constexpr_string_type>::type){}; \
  }())
