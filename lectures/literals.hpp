#ifndef META__LITERALS_HPP
#define META__LITERALS_HPP

#include "type_erasure/meta.hpp"
#include <cstddef>
#include <utility>

namespace compile_time_string {
inline namespace {
template <size_t I, size_t N>
constexpr auto sget(const char (&arr)[N]) {
  return arr[I];
}

template <char C>
using char_ = std::integral_constant<char, C>;

template <int I>
using int_ = std::integral_constant<int, I>;

template <char... chars>
struct string_;

template <size_t N, typename str_type>
struct to_string_helper {
  template <size_t index>
  struct char_at_ {
      using type = char_<str_type{}.char_at(index)>;
  };
  template <typename Index>
  using char_at = typename char_at_<Index{}>::type;
  using type = meta::map<meta::index_sequence<N>, char_at>;
};

template <typename... Ts>
meta::typelist<Ts...> types(Ts...) { return {}; }
} // anon namespace
} // compile_time_string

#define STRING_C(literal)                                              \
  ([] {                                                                \
    using compile_time_string::to_string_helper;                       \
    struct constexpr_string_type {                                     \
      constexpr auto char_at(size_t i) { return (literal)[i]; }        \
    };                                                                 \
    return (typename to_string_helper<sizeof((literal)),               \
                                      constexpr_string_type>::type){}; \
  }())

#endif
