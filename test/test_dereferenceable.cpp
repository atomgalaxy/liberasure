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

#include "erasure/erasure.hpp"
#include "erasure/feature/callable.hpp"
#include "erasure/feature/dereferenceable.hpp"
#include "erasure/feature/regular.hpp"

#include <cassert>
#include <memory>
#include <tuple>

struct can_const_deref {
  int operator*() const { return 1; }

  friend bool operator==(can_const_deref const &, can_const_deref const &) {
    return true;
  }
};

struct can_mutably_deref {
  int x = 0;
  int operator*() { return ++x; }

  friend bool operator==(can_mutably_deref const &, can_mutably_deref const &) {
    return true;
  }
};
struct can_both_deref {
  int x = 0;
  int operator*() { return ++x; }
  int operator*() const { return x; }

  friend bool operator==(can_both_deref const &, can_both_deref const &) {
    return true;
  }
};

void test_const_dereferenceable() {
  using erasure::make_any;
  using erasure::features::const_dereferenceable;
  using erasure::features::regular;

  auto x = make_any<regular, const_dereferenceable<int>>(can_const_deref{});
  assert(*x == 1);
}

void test_mutably_dereferenceable() {
  using erasure::make_any;
  using erasure::features::mutably_dereferenceable;
  using erasure::features::regular;

  auto x = make_any<regular, mutably_dereferenceable<int>>(can_mutably_deref{});
  assert(*x == 1);
  assert(*x == 2);
  assert(*x == 3);
}

void test_dereferenceable_and_mutably_dereferenceable_can_be_together() {
  using erasure::make_any;
  using erasure::features::const_dereferenceable;
  using erasure::features::mutably_dereferenceable;
  using erasure::features::regular;

  auto x = make_any<regular, mutably_dereferenceable<int>,
                    const_dereferenceable<int>>(can_both_deref{});
  auto const &y = std::as_const(x);
  assert(*x == 1);
  assert(*y == 1);
  assert(*x == 2);
}

int main() {
  test_const_dereferenceable();
  test_mutably_dereferenceable();
  test_dereferenceable_and_mutably_dereferenceable_can_be_together();
}
