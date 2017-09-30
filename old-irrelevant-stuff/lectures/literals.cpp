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

#include "literals.hpp"
#include "type_erasure/meta.hpp"
#include <type_traits>

auto options = meta::make_typelist(STRING_C("feature1"),
                                   STRING_C("feature2"),
                                   STRING_C("feature3"),
                                   STRING_C("feature4"));
auto uniqued_options = meta::unique<decltype(options)>{};

template <typename Key, typename Value>
struct key_value_pair {};

template <typename Pair, typename Needle>
struct key_value_pair_matches;
template <typename Key, typename Value, typename Needle>
struct key_value_pair_matches<key_value_pair<Key, Value>, Needle>
    : std::is_same<Needle, Key> {};

template <typename... KeyValuePairs>
struct typemap {
  using data = meta::typelist<KeyValuePairs...>;

  template <typename Key>
  auto constexpr operator[](Key) {
    return meta::find_first<data, key_value_pair_matches>{};
  }
};

template <typename Key, typename Value>
auto make_pair(Key, Value) -> key_value_pair<Key, Value> {
  return {};
}

template <typename... Key, typename... Value>
auto make_typemap(key_value_pair<Key, Value>...)
    -> typemap<key_value_pair<Key, Value>...> {
  return {};
}

int main() {
  auto my_map = make_typemap(make_pair(STRING_C("time"), bool_<true>{}),
                             make_pair(STRING_C("user_time", bool_<false>{})));
}
