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

#include <type_traits>

template <typename T, T v>
struct integral_constant {
  using value_type = T;

  static T constexpr value = v;

  constexpr operator T () const {
    return v;
  }
};

template <typename T, T v, typename U, U u>
constexpr auto operator+(integral_constant<T, v>, integral_constant<U, u>) {
  return integral_constant<decltype(v + u), v + u>();
}

template <typename T, T v, typename U, U u>
constexpr auto operator-(integral_constant<T, v> x, integral_constant<U, u> y)
    -> integral_constant<decltype(v - u), x.value - y.value> {
  return {};
}

template <int I>
using int_ = integral_constant<int, I>;

template <bool V>
using bool_ = integral_constant<bool, V>;

template <bool V>
constexpr auto operator!(bool_<V>) -> bool_<!V> { return {}; }

template <typename T>
struct type_ {
  using type = T;
};
template <typename T, typename U>
constexpr auto operator==(type_<T>, type_<U>)
    -> bool_<std::is_same<T, U>::value> {
  return {};
}
template <typename T, typename U>
constexpr auto operator!=(type_<T> x, type_<U> y) {
  return !(x == y);
}

template <typename... Ts>
struct list {};

template <typename List1, typename List2>
struct concatenate_two;

template <typename... Ts, typename... Us>
struct concatenate_two<list<Ts...>, list<Us...>> {
  using type = list<Ts..., Us...>;
};

template <typename List1, typename List2>
using concatenate_two_t = typename concatenate_two<List1, List2>::type;

using list1 = list<int, char, list<int>>;
using list2 = list<>;

using cactus = concatenate_two_t<list1, list2>;

// -- metafunction: concatenate --
template <typename... Lists>
struct concatenate;

template <>
struct concatenate<> { using type = list<>; };

template <typename... Ts>
struct concatenate<list<Ts...>> {
  using type = list<Ts...>;
};

template <typename... Ts, typename... Us, typename... Rest>
struct concatenate<list<Ts...>, list<Us...>, Rest...> {
  using type = typename concatenate<list<Ts..., Us...>, Rest...>::type;
};

template <typename... Lists>
using concatenate_t = typename concatenate<Lists...>::type;


int main() {
  auto y = int_<1>() + int_<2>();
  auto z = 1 + y;
  using type_of_y = decltype(y);
  static_assert(type_<decltype(z)>() == type_<int>(), "z is not int.");
  list<int, long, char, long, char> w;
}
