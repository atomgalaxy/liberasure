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

#include "debug/instrumented.hpp"

#include <memory>
#include <tuple>

int main() {
  using erasure::features::copy_assignable;
  using erasure::features::copy_constructible;
  using erasure::features::copyable;
  using erasure::features::movable;
  using erasure::features::move_assignable;
  using erasure::features::move_constructible;

  using dbg_util::instrumented;
  using erasure::any;
  using erasure::make_any;
  using std::make_tuple;
  using std::make_unique;

  {
    // movable does not require copyable
    auto x = make_any<movable>(make_unique<instrumented<int>>(5));
    ASSERT_AND_CLEAR_TRACE_IS(
        make_tuple(0, -1, dbg_util::operation::VALUE_CONSTRUCTION));
    auto y = std::move(x);
#ifdef NOCOMPILE_COPYABLE_TEST
    // requires copying, should not compile
    auto z = y;
#endif
  }

  // move_constructible invokes move construction
  {
    dbg_util::clear_trace();
    dbg_util::reset_numbering();

    auto x = make_any<move_constructible>(instrumented<int>{5});
    dbg_util::clear_trace(); // don't care about traces for this

    auto y = std::move(x);
    ASSERT_AND_CLEAR_TRACE_IS(
        make_tuple(2, 1, dbg_util::operation::MOVE_CONSTRUCTION));
  }

  // move_assignable invokes move assignment between same types
  {
    dbg_util::clear_trace();
    dbg_util::reset_numbering();

    any<movable> x{instrumented<int>{5}};
    any<movable> y{instrumented<int>{6}};
    dbg_util::clear_trace();

    y = std::move(x);
    ASSERT_AND_CLEAR_TRACE_IS(
        make_tuple(3, 1, dbg_util::operation::MOVE_ASSIGNMENT));
  }

  {
    dbg_util::clear_trace();
    dbg_util::reset_numbering();

    auto x = make_any<move_constructible>(instrumented<int>{5});
    ASSERT_AND_CLEAR_TRACE_IS(
        make_tuple(0, -1, dbg_util::operation::VALUE_CONSTRUCTION),
        make_tuple(1, 0, dbg_util::operation::MOVE_CONSTRUCTION),
        make_tuple(0, -1, dbg_util::operation::DESTRUCTION));
    decltype(x) y;
#ifdef NOCOMPILE_MOVABLE_TEST
    // requires move-assignment, should not compile
    y = std::move(x);
#endif
  }

  {
    dbg_util::clear_trace();
    dbg_util::reset_numbering();

    auto x1 = make_any<move_constructible>(instrumented<int>{5});
    auto x2 =
        make_any<copy_constructible, move_assignable>(instrumented<int>{5});
    auto x3 =
        make_any<move_constructible, copy_assignable>(instrumented<int>{5});
  }
}
