#include <type_traits>

// -- integral constant --
template <typename T, T v>
struct integral_constant {
  using value_type = T;
  using type = integral_constant;
  static T constexpr value = v;

  constexpr operator T () const {
    return v;
  }
};

// -- operators for integral constant --
// you can implement them this way...
template <typename T, T v, typename U, U u>
constexpr auto operator+(integral_constant<T, v>, integral_constant<U, u>) {
  return integral_constant<decltype(v + u), v + u>();
}

// or this way (see the difference in specification of return value)
template <typename T, T v, typename U, U u>
constexpr auto operator-(integral_constant<T, v> x, integral_constant<U, u> y)
    -> integral_constant<decltype(v - u), x.value - y.value> {
  return {};
}

// -- useful helpers
// -- int: TODO variable template
template <int I>
using int_ = integral_constant<int, I>;

template <int I>
int_<I> int_c{};

// -- bool: TODO variable template
template <bool V>
using bool_ = integral_constant<bool, V>;

template <bool V>
bool_<V> bool_c{};

template <bool V>
constexpr auto operator!(bool_<V>) -> bool_<!V> { return {}; }

// -- type.
template <typename T>
struct type_ {
  using type = T;
};
template <typename T>
type_<T> type_c{};

template <typename T, typename U>
constexpr auto operator==(type_<T>, type_<U>)
    -> bool_<std::is_same<T, U>::value> {
  return {};
}
template <typename T, typename U>
constexpr auto operator!=(type_<T> x, type_<U> y) {
  return !(x == y);
}
// -- constexpr operator< is not possible, as far as I know, and I tried a lot.

// -- typelist
template <typename... Ts>
struct list {};

// -- concatenate_two: not needed once we have concatenate
template <typename List1, typename List2>
struct concatenate_two;

template <typename... Ts, typename... Us>
struct concatenate_two<list<Ts...>, list<Us...>> {
  using type = list<Ts..., Us...>;
};

template <typename List1, typename List2>
using concatenate_two_t = typename concatenate_two<List1, List2>::type;

// test
namespace tests {
using list1 = list<int, char, list<int>>;
using list2 = list<>;

using cactus = concatenate_two_t<list1, list2>;
static_assert(type_c<cactus> == type_c<list<int, char, list<int>>>, "");
}

// -- concatenate --
// prototype
template <typename... Lists>
struct concatenate;

// concatenate with no arguments
template <>
struct concatenate<> { using type = list<>; };

// concatenate with one argument
template <typename... Ts>
struct concatenate<list<Ts...>> {
  using type = list<Ts...>;
};

// concatenate with at least two arguments
template <typename... Ts, typename... Us, typename... Rest>
struct concatenate<list<Ts...>, list<Us...>, Rest...> {
  using type = typename concatenate<list<Ts..., Us...>,
                                    Rest...>::type;
};

template <typename... Lists>
using concatenate_t = typename concatenate<Lists...>::type;

// -- TODAY, we go on from here.

// -- map
//
// -- foldr
// foldr acc (a b c) = 
// -- foldl
// foldl acc (a b c) = (((acc 'f' a) 'f' b) 'f' c)

template <template <typename, typename> class F,
          typename Start,
          typename List>
struct foldr;

template <template <typename, typename> class F,
          typename Start>
struct foldr<F, Start, list<>> {
  using type = Start;
};

template <template <typename, typename> class F,
          typename Start,
          typename Head,
          typename... Rest>
struct foldr<F, Start, list<Head, Rest...>> {
  using tmp = typename foldr<F, Start, list<Rest...>>::type;
  using type = F<Head, tmp>;
};

template <template <typename, typename> class F,
          typename Start,
          typename List>
struct foldl;

template <template <typename, typename> class F,
          typename Start>
struct foldl<F, Start, list<>> {
  using type = Start;
};

template <template <typename, typename> class F,
          typename Start,
          typename Head,
          typename... Rest>
struct foldl<F, Start, list<Head, Rest...>> {
  using accumulator = F<Start, Head>;
  using type =
      typename foldl<F, accumulator, list<Rest...>>::type;
};

// foldl f start (1, 2, 3, 4)
// ((((start + 1) + 2) + 3) + 4)

template <template <typename> class F, typename List>
struct map;

template <template <size_t, typename> class F,
          typename... Ts,
          size_t... Is>
struct map<F, list<Ts...>> {
  using type = list<F<Is, Ts>...>;
};

template <typename T, typename U>
struct sub;

template <typename T, T V, typename U, U W>
struct sub<integral_constant<T, V>, integral_constant<U, W>>
    : integral_constant<decltype(V - W), V - W> {};

template <typename T, typename U>
using sub_t = typename sub<T, U>::type;

template <template <typename, typename> class F,
          typename Start, typename List>
using foldr_t = typename foldr<F, Start, List>::type;

int main() {
  auto y = int_<1>() + int_<2>();
  auto z = 1 + y;
  using type_of_y = decltype(y);
  static_assert(type_<decltype(z)>() == type_<int>(), "z is not int.");
  list<int, long, char, long, char> w;

  auto diff =
      foldr_t<sub_t, int_<0>, list<int_<1>, int_<2>>>{};
  static_assert(sum == int_c<-1>, "");

  auto x = concatenate_t<list<int>,
                         list<char, int>,
                         list<void, char>,
                         list<long long>>{};
  static_assert(type_c<decltype(x)> ==
                    type_c<list<int, char, int, void,
                                char, long long>>,
                "");
}
