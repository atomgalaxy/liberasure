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

#pragma once

#include <type_traits>
#include <utility>

namespace erasure {
namespace meta {

/**
 * Basic type for storing lists of types.
 */
template <typename... Ts>
struct typelist {};
template <typename... Ts>
auto constexpr make_typelist(Ts...) -> typelist<Ts...> {
  return {};
}
template <typename... Ts, typename... Us>
constexpr auto operator==(typelist<Ts...> x, typelist<Us...> y) {
  return std::integral_constant<bool,
                                std::is_same<decltype(x), decltype(y)>{}>{};
};
template <typename... Ts, typename... Us>
constexpr auto operator!=(typelist<Ts...> x, typelist<Us...> y) {
  return std::integral_constant<bool, !(x == y)>{};
};

namespace detail {
/** typevariable for debugging: */
template <typename... Ts>
struct type_printer; // undefined
template <typename... Ts>
type_printer<Ts...> print_type;

/** is_typelist */
template <typename T>
struct is_typelist : std::false_type {};
template <typename... Ts>
struct is_typelist<typelist<Ts...>> : std::true_type {};

/** unbox */
template <typename F>
using _t = typename F::type;
/** rebox */
template <typename T>
struct type_ {
  using type = T;

  template <typename U, typename V>
  friend auto operator|(type_<U>, type_<V>) -> typelist<U, V> {
    return {};
  };
  template <typename U, typename V>
  friend constexpr auto operator==(type_<U>, type_<V>) {
    return _t<std::is_same<T, U>>{};
  }
};

/** and_ */
template <typename T, typename U>
struct and_
    : std::integral_constant<std::decay_t<decltype(T{} && U{})>, (T{} && U{})> {
};

/** or_ */
template <typename T, typename U>
struct or_
    : std::integral_constant<std::decay_t<decltype(T{} || U{})>, (T{} || U{})> {
};

/** not_ */ template <typename T>
struct not_ : std::integral_constant<bool, !T{}> {};

/** plus */
template <typename T, typename U>
struct plus
    : std::integral_constant<std::decay_t<decltype(T{} + U{})>, (T{} + U{})> {};

/** minus */
template <typename T, typename U>
struct minus
    : std::integral_constant<std::decay_t<decltype(T{} - U{})>, (T{} - U{})> {};

/** true and false predicates */
template <typename>
using true_predicate = std::true_type;
template <typename>
using false_predicate = std::false_type;

/** is_same import */
using std::is_same;
template <typename T, typename U>
using is_same_t = typename std::is_same<T, U>::type;

/** compose */
template <template <typename> class F, template <typename...> class G>
struct compose {
  template <typename... Args>
  struct f {
    using type = F<G<Args...>>;
  };
  template <typename... Args>
  using type = typename f<Args...>::type;
  template <typename A>
  using unary = typename f<A>::type;
  template <typename A, typename B>
  using binary = typename f<A, B>::type;
  template <typename A, typename B, typename C>
  using ternary = typename f<A, B, C>::type;
  template <typename... Args>
  using vararg = type<Args...>;
};

/** cons */
template <typename T, typename Typelist>
struct cons {
  static auto Typelist_is_this = print_type<Typelist>;
  static_assert(is_typelist<Typelist>{},
                "The typelist parameter should be a typelist.");
};
template <typename T, typename... Ts>
struct cons<T, typelist<Ts...>> {
  using type = typelist<T, Ts...>;
};
template <typename T, typename Typelist = typelist<>>
using cons_t = _t<cons<T, Typelist>>;

/** pair constructor */
template <typename T, typename U>
struct pair {
  using type = typelist<T, U>;
};
template <typename T, typename U>
using pair_t = _t<pair<T, U>>;

/** head */
template <typename Typelist>
struct head;
template <typename T, typename... Ts>
struct head<typelist<T, Ts...>> {
  using type = T;
};
template <typename Typelist>
using head_t = _t<head<Typelist>>;

/** tail */
template <typename Typelist>
struct tail;
template <typename T, typename... Ts>
struct tail<typelist<T, Ts...>> {
  using type = typelist<Ts...>;
};
template <typename Typelist>
using tail_t = _t<tail<Typelist>>;

/** take_1 */
template <typename Typelist>
struct take_1;
template <typename T, typename... Ts>
struct take_1<typelist<T, Ts...>> {
  using type = typelist<T>;
};
template <>
struct take_1<typelist<>> {
  using type = typelist<>;
};

template <typename Typelist>
using take_1_t = _t<take_1<Typelist>>;

/** concatenate */
template <typename... Typelists>
struct concatenate;

template <>
struct concatenate<> {
  using type = typelist<>;
};
template <typename... Ts>
struct concatenate<typelist<Ts...>> {
  using type = typelist<Ts...>;
};
template <typename... Ts1, typename... Ts2, typename... Typelists>
struct concatenate<typelist<Ts1...>, typelist<Ts2...>, Typelists...> {
  using type = _t<concatenate<typelist<Ts1..., Ts2...>, Typelists...>>;
};
template <typename... Typelists>
using concatenate_t = _t<concatenate<Typelists...>>;

/** flatten. */
template <typename T>
struct flatten {
  using type = typelist<T>;
};
template <typename... Ts>
struct flatten<typelist<Ts...>> {
  using type = concatenate_t<_t<flatten<Ts>>...>;
};
template <typename Typelist>
using flatten_t = _t<flatten<Typelist>>;

static_assert(typelist<>{} == flatten_t<typelist<>>{},
              "Empty typelist flattened is empty.");
static_assert(typelist<int, long>{} == flatten_t<typelist<int, long>>{},
              "A flat typelist is flat already.");
static_assert(
    typelist<int, char, long, unsigned>{} ==
        flatten_t<typelist<int, typelist<char, typelist<long>, unsigned>>>{},
    "Flatten should flatten recursively.");

template <typename T>
struct join;
template <typename... Ts>
struct join<typelist<Ts...>> {
  using type = concatenate_t<Ts...>;
};
template <typename Typelist>
using join_t = _t<join<Typelist>>;

/**
 * Map:
 */
template <template <typename...> class F, typename Typelist,
          typename... ExtraArgs>
struct map {
  // print what this is, because it's evidently not a typelist
  static auto const this_should_be_a_typelist = print_type<Typelist>;
};
template <template <typename...> class F, typename... Ts, typename... ExtraArgs>
struct map<F, typelist<Ts...>, ExtraArgs...> {
  using type = typelist<F<Ts, ExtraArgs...>...>;
};
template <template <typename...> class F, typename Typelist,
          typename... ExtraArgs>
using map_t = _t<map<F, Typelist, ExtraArgs...>>;

template <std::size_t I>
using size_t_ = std::integral_constant<std::size_t, I>;

template <std::size_t N>
struct make_index_sequence {
  using seq = std::make_index_sequence<N>;

  template <typename IndexSequence>
  struct helper;
  template <std::size_t... Is>
  struct helper<std::index_sequence<Is...>> {
    using type = typelist<size_t_<Is>...>;
  };

  using type = typename helper<seq>::type;
};
template <std::size_t N>
using make_index_sequence_t = _t<make_index_sequence<N>>;

/**
 * foldl
 *
 * foldl(f, acc, (a, b, c))
 */
template <template <typename, typename> class BinaryF, typename Acc,
          typename Typelist>
struct foldl {
  static auto Typelist_is_this = print_type<Typelist>;
  static_assert(is_typelist<Typelist>{},
                "The typelist parameter should be a typelist.");
};
template <template <typename, typename> class BinaryF, typename Acc>
struct foldl<BinaryF, Acc, typelist<>> {
  using type = Acc;
};
template <template <typename, typename> class BinaryF, typename Acc, typename T,
          typename... Rest>
struct foldl<BinaryF, Acc, typelist<T, Rest...>> {
  using r_type = BinaryF<Acc, T>;
  using type = _t<foldl<BinaryF, r_type, typelist<Rest...>>>;
};
template <template <typename, typename> class BinaryF, typename Acc,
          typename Typelist>
using foldl_t = _t<foldl<BinaryF, Acc, Typelist>>;
static_assert(foldl_t<pair_t, char, typelist<short, int, long>>{} ==
                  typelist<typelist<typelist<char, short>, int>, long>{},
              "foldl unit test");

/** foldr */
template <template <typename, typename> class BinaryF, typename Typelist,
          typename Acc>
struct foldr;
template <template <typename, typename> class BinaryF, typename Acc>
struct foldr<BinaryF, typelist<>, Acc> {
  using type = Acc;
};
template <template <typename, typename> class BinaryF, typename T,
          typename... Rest, typename Acc>
struct foldr<BinaryF, typelist<T, Rest...>, Acc> {
  using r_type = _t<foldr<BinaryF, typelist<Rest...>, Acc>>;
  using type = BinaryF<T, r_type>;
};
template <template <typename, typename> class BinaryF, typename Typelist,
          typename Acc>
using foldr_t = _t<foldr<BinaryF, Typelist, Acc>>;

static_assert(foldr_t<pair_t, typelist<char, short, int>, long>{} ==
                  typelist<char, typelist<short, typelist<int, long>>>{},
              "foldr unit test");
static_assert(type_<foldr_t<pair_t, typelist<>, long>>{} == type_<long>{},
              "foldr unit test");

template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
struct copy_if {
  template <typename T>
  using predicate = std::conditional_t<NaryPredicate<T, PredicateParams...>{},
                                       typelist<T>, typelist<>>;
  using type = join_t<map_t<predicate, Typelist>>;
};
template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
using copy_if_t = _t<copy_if<NaryPredicate, Typelist, PredicateParams...>>;

/* tests */
static_assert(copy_if_t<true_predicate, typelist<>>{} == typelist<>{}, "");
static_assert(copy_if_t<true_predicate, typelist<int>>{} == typelist<int>{},
              "");
static_assert(copy_if_t<false_predicate, typelist<int>>{} == typelist<>{}, "");
static_assert(copy_if_t<is_same, typelist<int, long>, int>{} == typelist<int>{},
              "");
static_assert(copy_if_t<is_same, typelist<long, int>, long>{} ==
                  typelist<long>{},
              "");
static_assert(copy_if_t<true_predicate, typelist<long, int>>{} ==
                  typelist<long, int>{},
              "");
static_assert(copy_if_t<false_predicate, typelist<long, int>>{} == typelist<>{},
              "");
/* test compose */
static_assert(copy_if_t<compose<not_, is_same>::type, typelist<int>, int>{} ==
                  typelist<>{},
              "");
static_assert(copy_if_t<compose<not_, is_same>::type, typelist<long>, int>{} ==
                  typelist<long>{},
              "");

/** copy if not */
template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
using copy_if_not_t = copy_if_t<compose<not_, NaryPredicate>::template type,
                                Typelist, PredicateParams...>;
template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
using copy_if_not =
    type_<copy_if_not_t<NaryPredicate, Typelist, PredicateParams...>>;

/** find_first */
template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
using find_first_t =
    head_t<copy_if_t<NaryPredicate, Typelist, PredicateParams...>>;
template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
using find_first =
    type_<find_first_t<NaryPredicate, Typelist, PredicateParams...>>;

/** find_first_not */
template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
using find_first_not_t =
    find_first_t<compose<not_, NaryPredicate>::template type, Typelist,
                 PredicateParams...>;
template <template <typename...> class NaryPredicate, typename Typelist,
          typename... PredicateParams>
using find_first_not =
    type_<find_first_not_t<NaryPredicate, Typelist, PredicateParams...>>;

/** unique */
template <typename Typelist, template <typename, typename> class Equals>
struct unique {
  template <typename Result, typename T>
  using include_if_not_in =
      std::conditional_t<typelist<>{} == copy_if_t<Equals, Result, T>{},
                         concatenate_t<Result, typelist<T>>, Result>;

  using type = foldl_t<include_if_not_in, typelist<>, Typelist>;
};
template <typename Typelist,
          template <typename, typename> class Equals = is_same>
using unique_t = _t<unique<Typelist, Equals>>;

static_assert(typelist<int, long, char>{} ==
                  unique_t<typelist<int, long, int, long, char, long>>{},
              "");

/** typelist length. */
template <typename Typelist>
struct len;
template <typename... Ts>
struct len<typelist<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};
template <typename Typelist>
using len_t = typename len<Typelist>::type;

template <typename S1, typename S2>
struct intersection {
  template <typename T>
  using predicate = take_1_t<copy_if_t<is_same, S2, T>>;
  using type = join_t<map_t<predicate, S1>>;
};
template <typename Set1, typename Set2>
using intersection_t = _t<intersection<Set1, Set2>>;

static_assert(typelist<char>{} ==
                  intersection_t<typelist<int, long, char>,
                                 typelist<std::size_t, char, long long>>{},
              "");
static_assert(typelist<>{} ==
                  intersection_t<typelist<int>, typelist<char, unsigned>>{},
              "");

template <typename Subset, typename Set>
struct is_subset_of {
  using type = is_same_t<len<Subset>, len<intersection_t<Subset, Set>>>;
};
template <typename Subset, typename Set>
using is_subset_of_t = _t<is_subset_of<Subset, Set>>;

template <typename T, typename Set>
using is_element_t = is_subset_of_t<typelist<T>, Set>;
template <typename T, typename Set>
using is_element = type_<is_element_t<T, Set>>;

static_assert(!is_element_t<int, typelist<>>{},
              "Empty arrays don't have anything.");
static_assert(!is_element_t<int, typelist<char>>{},
              "Wrong types don't register.");
static_assert(is_element_t<int, typelist<int, char>>{},
              "First element is fine.");
static_assert(is_element_t<int, typelist<char, int>>{},
              "Last element is fine.");
static_assert(is_element_t<int, typelist<long, int, char>>{},
              "Middle element is fine.");
static_assert(!is_element_t<unsigned int, typelist<long, int, char>>{},
              "Long typelist is fine.");

template <typename T, typename Set>
using is_not_element_t = not_<is_element_t<T, Set>>;
template <typename T, typename Set>
using is_not_element = type_<is_not_element_t<T, Set>>;

static_assert(is_not_element_t<int, typelist<>>{},
              "Empty arrays don't have anything.");
static_assert(is_not_element_t<int, typelist<char>>{},
              "Wrong types don't register.");
static_assert(!is_not_element_t<int, typelist<int, char>>{},
              "First element is fine.");
static_assert(!is_not_element_t<int, typelist<char, int>>{},
              "Last element is fine.");
static_assert(!is_not_element_t<int, typelist<long, int, char>>{},
              "Middle element is fine.");
static_assert(is_not_element_t<unsigned int, typelist<long, int, char>>{},
              "Long typelist is fine.");

template <template <typename> class Predicate, typename Typelist>
using any_t = foldl_t<or_, std::false_type, map_t<Predicate, Typelist>>;
template <template <typename> class Predicate, typename Typelist>
using any = type_<any_t<Predicate, Typelist>>;

template <template <typename> class Predicate, typename Typelist>
using all_t = foldl_t<and_, std::true_type, map_t<Predicate, Typelist>>;
template <template <typename> class Predicate, typename Typelist>
using all = type_<all_t<Predicate, Typelist>>;

template <template <typename, typename> class Equals, typename Typelist>
struct group_by;
template <template <typename, typename> class Equals, typename... Ts>
struct group_by<Equals, typelist<Ts...>> {
  using ts = typelist<Ts...>;
  using type = unique_t<typelist<copy_if_t<Equals, ts, Ts>...>>;
};
template <template <typename, typename> class Equals, typename Typelist>
using group_by_t = _t<group_by<Equals, Typelist>>;

static_assert(is_same_t<typelist<typelist<int, int>, typelist<long, long>>,
                        group_by_t<is_same, typelist<int, long, int, long>>>{},
              "General test.");

/** product_2 */
template <typename, typename>
struct product_2;

// base case - first list is empty
template <typename... Us>
struct product_2<typelist<>, typelist<Us...>> {
  using type = typelist<>;
};
// step - expand and recurse
template <typename T, typename... Ts, typename... Us>
struct product_2<typelist<T, Ts...>, typelist<Us...>> {
  using type = concatenate_t<typelist<typelist<T, Us>...>,
                             _t<product_2<typelist<Ts...>, typelist<Us...>>>>;
};
template <typename Typelist1, typename Typelist2>
using product_2_t = _t<product_2<Typelist1, Typelist2>>;

/* PRODUCT */
template <typename... Typelists>
struct product;
// product of zero products is empty
template <>
struct product<> {
  using type = typelist<>;
};
// product of one typelist is the typelist
template <typename... Ts>
struct product<typelist<Ts...>> {
  using type = typelist<typelist<Ts...>>;
};
// we have at least two typelists - just fold over them
template <typename... Ts, typename... Us, typename... Typelists>
struct product<typelist<Ts...>, typelist<Us...>, Typelists...> {
  template <typename PairOfListAndT>
  using unpack = concatenate_t<head_t<PairOfListAndT>, tail_t<PairOfListAndT>>;
  template <typename TL1, typename TL2>
  using produce = map_t<unpack, product_2_t<TL1, TL2>>;
  using type = foldl_t<produce, product_2_t<typelist<Ts...>, typelist<Us...>>,
                       typelist<Typelists...>>;
};
template <typename... Typelists>
using product_t = _t<product<Typelists...>>;

static_assert(product_t<>{} == typelist<>{}, "");
static_assert(product_t<typelist<int>>{} == typelist<typelist<int>>{}, "");
static_assert(product_t<typelist<int, int>>{} == typelist<typelist<int, int>>{},
              "");
static_assert(product_t<typelist<int>, typelist<>>{} == typelist<>{}, "");
static_assert(product_t<typelist<>, typelist<int>>{} == typelist<>{}, "");
static_assert(product_t<typelist<int>, typelist<long>>{} ==
                  typelist<typelist<int, long>>{},
              "");
static_assert(product_t<typelist<int, long>, typelist<int>>{} ==
                  typelist<typelist<int, int>, typelist<long, int>>{},
              "");
static_assert(product_t<typelist<int>, typelist<int, long>>{} ==
                  typelist<typelist<int, int>, typelist<int, long>>{},
              "");
static_assert(
    product_t<typelist<char, short>, typelist<int, long>,
              typelist<float, double>, typelist<unsigned, unsigned char>>{} ==
        typelist<typelist<char, int, float, unsigned>,
                 typelist<char, int, float, unsigned char>,
                 typelist<char, int, double, unsigned>,
                 typelist<char, int, double, unsigned char>,
                 typelist<char, long, float, unsigned>,
                 typelist<char, long, float, unsigned char>,
                 typelist<char, long, double, unsigned>,
                 typelist<char, long, double, unsigned char>,
                 typelist<short, int, float, unsigned>,
                 typelist<short, int, float, unsigned char>,
                 typelist<short, int, double, unsigned>,
                 typelist<short, int, double, unsigned char>,
                 typelist<short, long, float, unsigned>,
                 typelist<short, long, float, unsigned char>,
                 typelist<short, long, double, unsigned>,
                 typelist<short, long, double, unsigned char>>{},
    "");

template <typename T, typename U>
using copy_rvref_t = std::conditional_t<std::is_rvalue_reference_v<T>,
                                        std::add_rvalue_reference_t<U>, U>;
template <typename T, typename U>
using copy_lvref_t = std::conditional_t<std::is_lvalue_reference_v<T>,
                                        std::add_lvalue_reference_t<U>, U>;
template <typename T, typename U>
using copy_ref_t = copy_rvref_t<T, copy_lvref_t<T, U>>;
template <typename T, typename U>
using copy_const_t =
    std::conditional_t<std::is_const_v<std::remove_reference_t<T>>,
                       std::add_const_t<U>, U>;
template <typename T, typename U>
using copy_volatile_t =
    std::conditional_t<std::is_volatile_v<std::remove_reference_t<T>>,
                       std::add_volatile_t<U>, U>;
template <typename T, typename U>
using copy_cvref_t = copy_ref_t<T, copy_const_t<T, copy_volatile_t<T, U>>>;

template <typename T, typename U>
[[gnu::always_inline]] inline constexpr auto forward_like(U &&x) noexcept
    -> copy_cvref_t<T, std::remove_reference_t<U>> {
  return std::forward<copy_cvref_t<T, std::remove_reference_t<U>>>(x);
}

template <typename T, typename U>
[[gnu::always_inline]] inline constexpr auto forward_cast(U &&x) noexcept
    -> copy_cvref_t<U &&, std::remove_cvref_t<T>> {
  using out_type = copy_cvref_t<U &&, std::remove_cvref_t<T>>;
  return static_cast<out_type>(x);
}

} // namespace detail

/** INTERFACE LISTING */
using detail::compose;

using detail::concatenate;
using detail::concatenate_t;

using detail::cons;
using detail::cons_t;

using detail::copy_if;
using detail::copy_if_t;

using detail::copy_if_not;
using detail::copy_if_not_t;

using detail::find_first;
using detail::find_first_t;

using detail::find_first_not;
using detail::find_first_not_t;

using detail::flatten;
using detail::flatten_t;

using detail::foldl;
using detail::foldl_t;

using detail::foldr;
using detail::foldr_t;

using detail::head;
using detail::head_t;

using detail::tail;
using detail::tail_t;

using detail::take_1;
using detail::take_1_t;

using detail::is_element;
using detail::is_element_t;

using detail::is_not_element;
using detail::is_not_element_t;

using detail::is_same;
using detail::is_same_t;

using detail::is_subset_of;
using detail::is_subset_of_t;

using detail::join;
using detail::join_t;

using detail::map;
using detail::map_t;

using detail::unique;
using detail::unique_t;

using detail::any;
using detail::any_t;

using detail::all;
using detail::all_t;

using detail::make_index_sequence;
using detail::make_index_sequence_t;

using detail::group_by;
using detail::group_by_t;

using detail::product;
using detail::product_t;

using detail::and_;
using detail::false_predicate;
using detail::is_typelist;
using detail::minus;
using detail::not_;
using detail::or_;
using detail::plus;
using detail::true_predicate;

using detail::copy_const_t;
using detail::copy_cvref_t;
using detail::copy_ref_t;
using detail::forward_cast;
using detail::forward_like;

using detail::print_type;

} // namespace meta
} // namespace erasure
