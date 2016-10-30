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

#include <vector>
#include <string>
#include <utility>
#include <cassert>
#include "type_erasure/type_erasure.hpp"

#define IGNORE_UNUSED (void*) nullptr &&


struct empty {};

template <typename Base, typename MyBase = empty>
struct quacker : MyBase {
  std::string quack() {
    auto t = static_cast<Base const&>(*this).times();
    std::string x;
    for (int i = 0; i < t; ++i) {
      x += "quack! ";
    }
    return x;
  }
};

template <typename Base, typename MyBase = empty>
struct howler : MyBase {
  std::string howl() {
    auto t = static_cast<Base const&>(*this).times();
    std::string x;
    for (int i = 0; i < t; ++i) {
      x += "wooooo! ";
    }
    return x;
  }
};

struct B : quacker<B, howler<B>> {
  int times() const { return 2; }
};

struct foo {
  std::unique_ptr<std::string> y;
  std::string x;
  foo(foo const& other)
      : y(other.y ? std::make_unique<std::string>(*other.y) : nullptr)
      , x(other.x) {}
  foo(foo&&) = default;
  foo() = default;
  foo& operator=(foo&&) = default;
  foo& operator=(foo const& other) {
    y = other.y ? std::make_unique<std::string>(*other.y) : nullptr;
    x = other.x;
    return *this;
  }
  friend bool operator==(foo const& x, foo const& y) {
    return x.x == y.x && (x.y && y.y &&  *x.y == *y.y);
  }
};

int main() {
  foo x;
  foo y(x);
  foo z(std::move(x));
  x = y;
  z = std::move(y);
  IGNORE_UNUSED z == x;

  B b;
  b.quack();
  b.howl();

}

