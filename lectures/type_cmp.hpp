#include <type_traits>
#include <array>
#include <utility>
#include <string>

namespace variadic_toolbox {
template <unsigned count,
          template <unsigned...> class meta_functor,
          unsigned... indices>
struct apply_range {
  typedef typename apply_range<count - 1, meta_functor, count - 1, indices...>::
      result result;
};

template <template <unsigned...> class meta_functor, unsigned... indices>
struct apply_range<0, meta_functor, indices...> {
  typedef typename meta_functor<indices...>::result result;
};
}

namespace compile_time {
template <char... str>
struct string {
  static constexpr const char value[sizeof...(str) + 1] = {str..., '\0'};
  operator std::string() { return {value}; }
};

template <char... str>
constexpr const char string<str...>::value[sizeof...(str) + 1];

template <typename lambda_str_type>
struct to_string {
  template <typename T>
  struct produce;

  template <size_t... Indices>
  struct produce<std::index_sequence<Indices...>> {
    using type = string<lambda_str_type{}.value[Indices]...>;
  };
};
}

template <bool V>
using bool_ = std::integral_constant<bool, V>;

template <char const* V>
using char_p_ = std::integral_constant<char const*, V>;

template <char... str>
struct string {
  static constexpr const char chars[sizeof...(str)+1] = {str..., '\0'};
};
template <char... str>
constexpr const char string<str...>::chars[sizeof...(str)+1];

constexpr char index(char const* const str, size_t i) {
  return str[i];
}

template <size_t... Indices>
auto constexpr index(char const* str, std::index_sequence<Indices...>) {
  return compile_time::string<str[Indices]...>{};
}

template <typename T>
struct type {
  static constexpr auto cmp_name() {
    auto constexpr index_sequence =
        std::make_index_sequence<sizeof("FOO")>();
    return index("FOO", index_sequence);
  }
};

constexpr bool less_than(char const* str1, char const* str2) {
  for (; *str1 && *str2; ++str1, ++str2) {
    if (*str1 < *str2) {
      return true;
    }
  }
  return false;
};

template <typename T, typename U>
constexpr auto lt(type<T> x, type<U> y) {
  return bool_<less_than(x.cmp_name(), y.cmp_name())>{};
}

template <typename T, typename U>
constexpr auto operator<(type<T> x, type<U> y)
{
  return lt(x, y);
}


//static_assert(cmp<type<int>, type<char>>::value, "foo");
