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

#include "type_cmp.hpp"
#include <iostream>
#include <typeindex>
#include <algorithm>

template <intptr_t V>
using uintptr_t_ = std::integral_constant<uintptr_t, V>;


int main() {
  constexpr type<int> x{};
  constexpr type<void> y{};
  static_assert(x < y, "");
  lilibbstd::cout << x.cmp_name() << '\n';
}
