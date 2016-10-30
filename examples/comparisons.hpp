#ifndef A9_CHAIN_COMPARISONS_HPP
#define A9_CHAIN_COMPARISONS_HPP

/**
 * This file demonstrates a use for so-called "expression templates".
 *
 * It does so by implementing a grammar of comparisons, and providing a
 * 'poisoning' object 'cmp' that allows one to 'enable' this grammar for a
 * specific expression, say, inside an 'if' statement.
 *
 * This grammar strives to enable chained comparisons as they are understood in
 * mathematics and python:
 * 1 < x <= y < 8, for instance.
 *
 * The abstract syntax tree (AST) is built out of two kinds of nodes; the leaves
 * are of kind bound<T>, and the inner nodes are comparison<T, U, Operator>.
 *
 * All tree nodes support the following interface:
 * - left_leaf_type and right_leaf_type: these give you the value type of the
 *   left and right leaves of the subtree, respectively.
 *   - For leaves, these both give the type of the value held in the leaf.
 *   - For inner nodes, they give the type of the value contained in their
 *     left-most or right-most leaf, recursively.
 * - left_leaf() and right_leaf() methods that give the value of the leaf in the
 *   right-most and left-most leaf nodes, respectively.
 * - value() 'evaluates' the node. For leaves, this means that it just returns
 *   'value'. For inner nodes, it evaluates the operator with the values of its
 *   children and returns the result.
 * - inner nodes:
 *   - operator bool() - this triggers automatic evaluation inside if statements
 * - leaf nodes:
 *   - implicit conversion to their value type, so that it can be used in
 *     places where that makes sense.
 *
 */

#define OVERLOAD_EQUALITY 0 // disable overloading equality

#include <type_traits>
#include <utility>

namespace comparison_chaining {

/**
 * We need to be able to associate a comparison operator with a type.
 *
 * Unfortunately, std::less and friends require that the types be the same, and
 * we don't want that, so we have to define our own. These become template
 * arguments to the comparison<> object, as the 'Operator' argument.
 */
namespace {
/* put them into the anon namespace so that the compiler can elide the empty
 * objects (less, less_or_equal, etc.) later. This is possible because the
 * anonymous namespace 'hides' the symbols from other compilation units, thus
 * informing the compiler that no compilation unit will need these objects.
 *
 * In other words, being in the anonymous namespace is roughly the same as
 * marking these 'static'.
 */

/* Notes on implementation:
 * - return types: making the return types decltype(auto) means we return
 *   *exactly* the type of the expression that is executed.
 * - Taking parameters by universal reference and std::forward'ing them means
 *   that comparison operators can take them whatever way they want, be it by
 *   const ref or not.
 */

struct less_t {
  template <typename T, typename U>
  constexpr decltype(auto) operator()(T&& x, U&& y) const {
    return std::forward<T>(x) < std::forward<U>(y);
  }
} less;

struct less_or_equal_t {
  template <typename T, typename U>
  constexpr decltype(auto) operator()(T&& x, U&& y) const {
    return std::forward<T>(x) <= std::forward<U>(y);
  }
} less_or_equal;

struct greater_t {
  template <typename T, typename U>
  constexpr decltype(auto) operator()(T&& x, U&& y) const {
    return std::forward<T>(x) > std::forward<U>(y);
  }
} greater;

struct greater_or_equal_t {
  template <typename T, typename U>
  constexpr decltype(auto) operator()(T&& x, U&& y) const {
    return std::forward<T>(x) >= std::forward<U>(y);
  }
} greater_or_equal;

struct equal_t {
  template <typename T, typename U>
  constexpr decltype(auto) operator()(T&& x, U&& y) const {
    return std::forward<T>(x) == std::forward<U>(y);
  }
} equal;

struct not_equal_t {
  template <typename T, typename U>
  constexpr decltype(auto) operator()(T&& x, U&& y) const {
    return std::forward<T>(x) != std::forward<U>(y);
  }
} not_equal;

struct and_t {
  template <typename T, typename U>
  constexpr decltype(auto) operator()(T&& x, U&& y) const {
    return std::forward<T>(x) && std::forward<U>(y);
  }
} and_;

} // end anon

/**
 * Represents the leaf of the AST.
 */
template <typename T>
struct bound {
  T v;
  using left_leaf_type = bound;
  using right_leaf_type = bound;

  constexpr T value() const { return v; }
  constexpr bound left_leaf() const { return *this; }
  constexpr bound right_leaf() const { return *this; }
  constexpr explicit operator T() const { return v; }

  static_assert(
      !std::is_same<std::decay_t<T>, bool>{},
      "bound<bool> cannot be instantiated. Comparisons with bool are usually a "
      "sign of wrong operator precedence, where a part of the expression was "
      "not captured correctly. Make sure to poison the parenthesized part of "
      "the expression with cmp().");
};
template <typename T>
auto constexpr make_bound(T x) -> bound<T> {
  return {x};
}

/**
 * The inner node of the AST.
 *
 * T and U are either another comparison<> or a bound<>.
 * Operator is the operator the comparison executes, from the set defined in the
 * anon namespace at the beginning of the file.
 */
template <typename T, typename U, typename Operator>
struct comparison {
  using left_leaf_type = typename T::left_leaf_type;
  using right_leaf_type = typename U::right_leaf_type;
  T l;
  U r;
  constexpr auto value() const { return Operator{}(l.value(), r.value()); }
  constexpr auto left_leaf() const { return l.left_leaf(); }
  constexpr auto right_leaf() const { return r.right_leaf(); }
  constexpr explicit operator bool() const { return value(); }
};
/**
 * Make a comparison object from a bound. Note that for Op, only the type, not
 * the value, is used.
 */
template <typename T, typename U, typename Op>
auto constexpr make_comparison(T x, U y, Op) -> comparison<T, U, Op> {
  return {x, y};
};
template <typename T, typename U>
auto constexpr make_constraint_pair(T x, U y) {
  return make_comparison(x, y, and_);
}
struct poisoner {
  template <typename T>
  constexpr auto operator%(T x) const -> bound<T> {
    return {x};
  }
  template <typename T>
  constexpr auto operator()(T x) const {
    return *this % x;
  }
};
/**
 * The grammar introducer. Use either as cmp % 5 < 6 <= 7 or cmp(5) < 6 < 7.
 */
static const poisoner cmp{};

/**
 * If, for some reason, you want to overload equality, change the 'config'
 * define at the top of the file to 1.
 *
 * The way it works: if OVERLOAD_EQUALITY is true, the FULL_COMPLEMENT macro
 * will include the code for == and != as well. Overloading equality has turned
 * out to be a bad idea because of operator precedence, so the default is false.
 */
#if OVERLOAD_EQUALITY
#define EQUALITY(MACRO) \
  MACRO(==, equal);     \
  MACRO(!=, not_equal); \
  static_assert(true, "require semicolon for macro")
#else
#define EQUALITY(MACRO) static_assert(true, "require semicolon for macro")
#endif

/**
 * Call MACRO with the arguments of (operator, functor-that-implements-it) for
 * each of <, <=, >, >= and, if OVERLOAD_EQUALITY, == and !=.
 *
 * This is a splendid example of a higher-order macro.
 *
 * The point is to not screw up and forget a specific operator, so we make a
 * macro that will call the macro we give it with all the appropriate
 * arguments, thus creating the "full complement" of comparison operators every
 * time.
 *
 * It is undefined at the end of the file, since it is only needed to output a
 * bunch of code, once.
 */
#define FULL_COMPLEMENT(MACRO) \
  MACRO(<, less);              \
  MACRO(<=, less_or_equal);    \
  MACRO(>, greater);           \
  MACRO(>=, greater_or_equal); \
  EQUALITY(MACRO);             \
  static_assert(true, "require semicolon for macro")

/*
 * The following groups of code are all structured in the same way:
 * - define an operator-generating macro
 * - call FULL_COMPLEMENT with it to generate the full complement of operators
 *   it represents
 * - undef the macro to stop polluting the namespace.
 *
 * The functions we must generate are:
 * - "poisoning" operators between a bound and anything (BOUND_POISON_RHS)
 * - "poisoning" operators between anything and a bound (BOUND_POISON_LHS)
 * - "poisoning" operators between comparison and anything
 *   (COMPARISON_POISON_RHS)
 * - "poisoning" operators between anything and comparison
 *   (COMPARISON_POISON_LHS)
 * - bound to bound comparison operators (BOUNDS)
 * - comparison to bound (COMPARISON_VS_BOUND)
 * - boound_t vs comparison (BOUND_VS_COMPARISON)
 * - comparison to comparison operators (COMPARISONS)
 *
 * These are all given below, in order.
 */
/**
 * "Poison" an arbitrary right hand side value that gets compared with a
 * bound.
 * This doesn't do the comparison - instead, it 'poisons' the right-hand side
 * (y) to be a bound as well, and dispatches to an operator generated by
 * BOUNDS.
 */
#define BOUND_POISON_RHS(op, f)                 \
  template <typename T, typename U>             \
  auto constexpr operator op(bound<T> x, U y) { \
    return x op make_bound(y);                  \
  }                                             \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(BOUND_POISON_RHS);
#undef BOUND_POISON_RHS

/** Same as BOUND_POISON_RHS, but for the left-hand side. */
#define BOUND_POISON_LHS(op, f)                 \
  template <typename T, typename U>             \
  auto constexpr operator op(T x, bound<U> y) { \
    return make_bound(x) op y;                  \
  }                                             \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(BOUND_POISON_LHS);
#undef BOUND_POISON_LHS

/**
 * "Poison" an arbitrary right-hand side value that gets compared with a
 * comparison object.
 * This doesn't do the comparison - instead, it makes 'y' be a bound, and
 * dispatches to the relevant bound-vs-comparison operator.
 */
#define COMPARISON_POISON_RHS(op, f)                          \
  template <typename T, typename U, typename Cmp, typename W> \
  auto constexpr operator op(comparison<T, U, Cmp> x, W y) {  \
    return x op make_bound(y);                                \
  }                                                           \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(COMPARISON_POISON_RHS);
#undef COMPARISON_POISON_RHS

/** Same as COMPARISON_POISON_RHS, but for the left-hand side. */
#define COMPARISON_POISON_LHS(op, f)                          \
  template <typename T, typename U, typename Cmp, typename W> \
  auto constexpr operator op(W x, comparison<T, U, Cmp> y) {  \
    return make_bound(x) op y;                                \
  }                                                           \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(COMPARISON_POISON_LHS);
#undef COMPARISON_POISON_LHS

/**
 * Operators between bound objects generate comparison objects.
 * This is where the function's type gets baked into the comparison object's 3rd
 * template parameter.
 */
#define BOUNDS(op, f)                                  \
  template <typename T, typename U>                    \
  auto constexpr operator op(bound<T> x, bound<U> y) { \
    return make_comparison(x, y, f);                   \
  }                                                    \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(BOUNDS);
#undef BOUNDS

/**
 * Comparisons between bound objects and comparison objects generate
 * comparison objects. This is the comparison-on-the-left version.
 */
#define COMPARISON_VS_BOUND(op, f)                                    \
  template <typename T1, typename U1, typename Cmp, typename T>       \
  auto constexpr operator op(comparison<T1, U1, Cmp> x, bound<T> y) { \
    return make_constraint_pair(x, x.right_leaf() op y);              \
  }                                                                   \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(COMPARISON_VS_BOUND);
#undef COMPARISON_VS_BOUND

/**
 * Same as COMPARISON_VS_BOUND, but the comparison-on-the-right version.
 */
#define BOUND_VS_COMPARISON(op, f)                                    \
  template <typename T1, typename U1, typename Cmp, typename T>       \
  auto constexpr operator op(bound<T> x, comparison<T1, U1, Cmp> y) { \
    return make_constraint_pair(x op y.left_leaf(), y);               \
  }                                                                   \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(BOUND_VS_COMPARISON);
#undef BOUND_VS_COMPARISON

/**
 * And finally, the comparison operators between comparisons... these generate
 * comparisons.
 */
#define COMPARISONS(op, f)                                            \
  template <typename T1,                                              \
            typename U1,                                              \
            typename T2,                                              \
            typename U2,                                              \
            typename Cmp1,                                            \
            typename Cmp2>                                            \
  auto constexpr operator op(comparison<T1, U1, Cmp1> x,              \
                             comparison<T2, U2, Cmp2> y) {            \
    return make_constraint_pair(                                      \
        x, make_constraint_pair(x.right_leaf() op y.left_leaf(), y)); \
  }                                                                   \
  static_assert(true, "require semicolon for macro")
FULL_COMPLEMENT(COMPARISONS);
#undef COMPARISONS

/* we are ready to clean up and undef everything that was needed till now. */
#undef FULL_COMPLEMENT
#undef EQUALITY
#undef OVERLOAD_EQUALITY

}

#endif
