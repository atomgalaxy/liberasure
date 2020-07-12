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

#include "erasure/feature/callable.hpp"
#include "erasure/feature/regular.hpp"

#include "erasure/erasure.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <tuple>

struct can_const_call {
  int operator()() const { return 1; }
  int operator()(int) const { return 2; }

  friend bool operator==(can_const_call const &, can_const_call const &) {
    return true;
  }
};

struct can_mutably_call {
  int x = 0;
  int operator()() { return ++x; }
  int operator()(int) { return --x; }

  friend bool operator==(can_mutably_call const &, can_mutably_call const &) {
    return true;
  }
};

void test_const_callable() {
  using erasure::make_any;
  using erasure::make_any_like;
  using erasure::features::buffer_size;
  using erasure::features::callable;
  using erasure::features::copyable;
  using erasure::features::regular;

  auto x = make_any<regular, callable<int() const>, callable<int(int) const>,
                    buffer_size<8 + 8>>(can_const_call{});
  assert(x() == 1);
  assert(x(1) == 2);
  can_const_call cc{};
  auto y = make_any_like<decltype(x)>(std::ref(cc));
  assert(y(1) == 2);
}

void test_mutably_callable() {
  using erasure::make_any;
  using erasure::features::callable;
  using erasure::features::regular;

  auto x = make_any<regular, callable<int()>, callable<int(int)>>(
      can_mutably_call{});
  assert(x() == 1);
  assert(x() == 2);
  assert(x() == 3);
  assert(x(1) == 2);
  assert(x(1) == 1);
}

int main() {
  test_const_callable();
  test_mutably_callable();
}
