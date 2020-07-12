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
#include "erasure/feature/dereferenceable.hpp"
#include "erasure/feature/less_than_comparable.hpp"
#include "erasure/feature/ostreamable.hpp"
#include "erasure/feature/regular.hpp"
#include "erasure/feature/value_equality_comparable.hpp"

#include "debug/atom.hpp"
#include "debug/instrumented.hpp"
#include "debug/unique_string.hpp"
#include "erasure/erasure.hpp"

#include <cassert>
#include <cstdlib>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>

int main() {
  using namespace erasure;
  using namespace erasure::features;
  using dbg_util::instrumented;
  using dbg_util::make_instrumented;

  { // test making a regular with make_any
    auto x = make_any<regular>(5);
    auto px = target<int>(x); // check whether x contains an int
    assert(px);
    assert(*px == 5);
  }

  {
    auto x = make_any<regular, equality_comparable_with<int, std::string>>(5);
    assert(x == 5);
    assert(x == 5ul);
    assert(x != 4);
    assert(4 != x);
    assert(5 == x);
    assert(x != std::string("I'm a char*"));
  }

  {
    auto x = make_any<move_constructible>(5);
    auto y = make_any<copy_constructible>(5);
  }

  {
    using dbg_util::operation;
    using std::make_tuple;
    dbg_util::clear_trace(); // clean up
    dbg_util::reset_numbering();
    auto x = make_instrumented(5);
    {
      ASSERT_AND_CLEAR_TRACE_IS(
          make_tuple(0, -1, operation::VALUE_CONSTRUCTION));
    }

    assert(x == x); // equals works
    { ASSERT_AND_CLEAR_TRACE_IS(make_tuple(0, 0, operation::EQUALS)); }

    auto y = make_instrumented(std::string("abcd"));
    {
      ASSERT_AND_CLEAR_TRACE_IS(
          make_tuple(1, -1, operation::VALUE_CONSTRUCTION));
    }
    assert(y == y); // equals on strings works
    { ASSERT_AND_CLEAR_TRACE_IS(make_tuple(1, 1, operation::EQUALS)); }

    any<regular> rx = make_any<regular>(x);
    {
      dbg_util::print_trace(std::cerr);
      ASSERT_AND_CLEAR_TRACE_IS(make_tuple(2, 0, operation::COPY_CONSTRUCTION));
    }

    std::cerr << "rx is int: " << (target<decltype(x)>(rx) ? "true" : "false")
              << "\n";
    assert(target<decltype(x)>(rx));
    assert(rx == rx); // rx's equals works
    std::cerr << "sizeof rx: " << sizeof(rx) << "\n";
    std::cerr << "sizeof *rx.value: " << erasure::debug::model_size(rx) << "\n";
    std::cerr << "rx is string: "
              << (target<decltype(y)>(rx) ? "true" : "false") << "\n";
    assert(!target<decltype(y)>(rx));

    std::cerr << "constructing ry from 'abcd'\n";
    auto ry = make_any<regular>(y);
    assert(ry == ry); // ry's equals works
    std::cerr << "ry is int: " << (target<decltype(x)>(ry) ? "true" : "false")
              << "\n";
    assert(!target<decltype(x)>(ry));
    std::cerr << "ry is string: "
              << (target<decltype(y)>(ry) ? "true" : "false") << "\n";
    assert(target<decltype(y)>(ry));

    std::cerr << "copy-constructing rz from rx\n";
    auto rz = rx;
    assert(rz == rx);

    std::cerr << "move-constructing rw from rz\n";
    any<regular> rw{std::move(rz)};

    std::cerr << "move-assigning rz = move(rw)\n";
    rz = std::move(rw);

    std::cerr << "copy-assigning rw = rz\n";
    rw = rz;

    std::cerr << "move-assigning rw = std::move(rz)\n";
    rw = std::move(rz);

    std::cerr << "copy-assigning rw = ry\n";
    rw = ry;

    std::cerr << "copy-assigning rz = ry\n";
    rz = ry;
    auto sptr = target<instrumented<std::string>>(rz);
    std::cerr << "rz is string: " << (sptr != nullptr ? "true" : "false")
              << "\n";

    std::cerr << "comparing rz and ry\n";
    std::cerr << "result: " << (rz == ry ? "true" : "false") << "\n";

    std::cerr << "copy-assigning rw = ry\n";
    rw = ry;

    std::cerr << "done, cleaning up.\n";
  }

  {
    // movable
    std::cerr << "Creating p1\n";
    auto p1 =
        make_instrumented(unique_string{std::make_unique<std::string>("p1")});
    std::cerr << "Creating p2\n";
    auto p2 =
        make_instrumented(unique_string{std::make_unique<std::string>("p2")});
    std::cerr << "Creating movable m1 from p1\n";
    auto m1 = make_any<movable>(std::move(p1));
    std::cerr << "Move-constructing m2 from p2.\n";
    auto m2 = make_any<movable>(std::move(p2));
    std::cerr << "Move-constructing m1 from m2.\n";
    m1 = std::move(m2);
  }

  {
    int i = 0;
    auto f = make_any<function<int(long)>>([pi = &i](long) { return ++*pi; });
    auto f1 = f;
    auto result = f(5);
    assert(i == 1);
    assert(result == 1);
    auto result1 = f(6);
    assert(i == 2);
    assert(result1 == 2);
  }

  {
    int i = 0;
    auto f = make_any<function<void()>>([pi = &i]() { ++*pi; });
    auto f1 =
        make_any<callable<void()>, move_constructible, copy_constructible>(
            [pi = &i]() { ++*pi; });
    f();
    assert(i == 1);
    std::cerr << "sizeof f: " << sizeof(f) << "\n";
  }

  {
    auto x = make_any<swappable, move_constructible, movable>(
        make_instrumented(std::string("Swappable 1")));
    auto y = make_any<swappable, move_constructible, movable>(
        make_instrumented(std::string("Swappable 2")));
    auto z = make_any<swappable, move_constructible, movable>(
        std::string("Swappable 3")); // different type
    auto w =
        make_any<swappable, move_constructible, movable>(make_instrumented(78));
    auto r = make_any<swappable, move_constructible, movable>(
        make_instrumented(79ul));
    swap(x, y); // should use ADL swap
    swap(x, z); // should use pointer-based swap
    swap(z, w); // should use non-internal swap
    swap(r, z); // should use std::swap.
  }

  {
    using std::swap;

    auto x = make_any<move_constructible, movable>(
        make_instrumented(std::string("Swappable 4")));
    auto y = make_any<move_constructible, movable>(
        make_instrumented(std::string("Swappable 5")));
    auto z = make_any<move_constructible, movable>(
        std::string("Swappable 6")); // different type
    auto w = make_any<move_constructible, movable>(make_instrumented(80));
    auto r = make_any<move_constructible, movable>(make_instrumented(81ul));

    swap(x, y); // should use ADL swap
    swap(x, z); // should use pointer-based swap
    swap(z, w); // should use non-internal swap
    swap(r, z); // should use std::swap.
  }

  {
    auto x = make_any<regular, ostreamable, buffer_size<16>>(5);
    auto y =
        make_any<regular, ostreamable, buffer_size<16>>(std::string("foo"));
    std::stringstream ss;
    ss << x << ' ' << y << '\n';
    x = y;
    ss << x << '\n';
    assert(ss.str() == "5 foo\nfoo\n");
  }

  { // less-then comparable
    auto const x = make_any<regular, features::less_than_comparable>(5);
    auto const y = make_any<regular, features::less_than_comparable>(7);
    assert(x < y);
    assert(y > x);
    assert(x <= y);
    assert(y >= x);
  }

  {
    unique_string x{std::make_unique<std::string>("foo")};
    auto const y = make_any<regular, features::ostreamable>(std::ref(x));

    std::stringstream ss;
    ss << y << '\n';
    assert(ss.str() == "foo\n");
  }
}
