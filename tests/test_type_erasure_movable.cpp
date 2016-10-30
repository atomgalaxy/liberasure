#include "type_erasure/type_erasure.hpp"

#include "debug/instrumented.hpp"

#include <memory>
#include <tuple>

int main() {
  using type_erasure::features::move_constructible;
  using type_erasure::features::move_assignable;
  using type_erasure::features::movable;
  using type_erasure::features::copyable;
  using type_erasure::features::copy_constructible;
  using type_erasure::features::copy_assignable;

  using type_erasure::make_any;
  using std::make_unique;
  using std::make_tuple;
  using dbg_util::instrumented;
  using type_erasure::any;

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
        make_tuple(0, -1, dbg_util::operation::DESTRUCTION)
        );
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
