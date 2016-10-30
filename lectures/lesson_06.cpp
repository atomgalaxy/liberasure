#include <string>
#include <tuple>

#include "../type_erasure/meta.hpp"

template <bool Value>
using bool_ = std::integral_constant<bool, Value>;

template <size_t start, size_t... Is>
auto indices_between_helper(std::index_sequence<Is...>)
    -> std::index_sequence<Is + start...> {}

template <size_t start, size_t end>
auto indices_between() -> decltype(
    indices_between_helper<start>(std::make_index_sequence<end - start>{})) {}

template <size_t I, typename... Ts, typename Value, size_t... IsBefore,
         size_t... IsAfter>
auto set_helper(std::tuple<Ts...> t, Value v, std::index_sequence<IsBefore...>,
    std::index_sequence<IsAfter...>) {
  return std::make_tuple(std::get<IsBefore>(t)..., v, std::get<IsAfter>(t)...);
}

template <size_t I, typename... Ts, typename Value>
auto set(std::tuple<Ts...> t, Value v) {
  return set_helper<I>(
      t, v, indices_between<0, I>(), indices_between<I + 1, sizeof...(Ts)>());
}

template <typename... Ts, size_t... Is>
auto from_tuple_helper(std::tuple<Ts...> t, std::index_sequence<Is...>);
template <typename... Ts>
auto from_tuple(std::tuple<Ts...> t) {
  return from_tuple_helper(t, std::index_sequence_for<Ts...>());
}

template <typename T0 = bool_<true>,
          typename T1 = bool_<true>>
struct config {
  T0 path_info;
  T1 time;
  auto to_tuple() const {
    return std::make_tuple(path_info, time);
  }
  template <typename T>
  auto set_path_info(T x) const {
    return from_tuple(set<0>(to_tuple(), x));
  }
  template <typename T>
  auto set_time(T x) const {
    return from_tuple(set<1>(to_tuple(), x));
  }
};

template <typename... Ts, size_t... Is>
auto from_tuple_helper(std::tuple<Ts...> t, std::index_sequence<Is...>) {
  return config<Ts...>{std::get<Is>(t)...};
}


template <typename Config>
int parse(Config c) {
  if (c.path_info) {
    //...
  }
}


int main() {
  auto t = std::make_tuple(1,2,3,4);
  auto s = set<1>(t, "blah");
  auto d = config<>{}.set_path_info(true).set_time(bool_<false>{});
}
