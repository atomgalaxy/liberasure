// TYPE ERASURE, try III

/*
 * 1) motivation
 * The concept of a property is *something with an operator[](size_t index)*.
 * Today, we will examine int properties, that is, things with an
 * <code>
 * int operator[](size_t index) const.
 * </code>
 *
 * 2) solution
 * 3) usage
 */
#include "unsigned_property.hpp"

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <random>

/**
 * Walk through all of the consecutive pairs of items between first and last
 * and call f with the values of those elements.
 *
 * @return a std::pair of (iterator-before-last, f)
 * Always return what you compute.
 */
template <typename ConstForwardIterator, typename BinaryOperation>
auto for_each_consecutive_pair(ConstForwardIterator first,
                               ConstForwardIterator const last,
                               BinaryOperation&& f)
    -> std::pair<ConstForwardIterator, BinaryOperation> {
  auto second = first;
  ++second;
  while (second != last) {
    f(*first, *second);  // call on values
    first = second;      // go forward
    ++second;            // for both
  }
  return std::make_pair(first, std::move(f));
}

/** read n items from cin. */
template <typename T>
auto read_n(size_t n) -> std::vector<T> {
  auto result = std::vector<T>(n);
  for (auto& v : result) {
    std::cin >> v;
  }
  return result;
}

/** read items from cin until end of file. */
template <typename T>
auto read_till_end() -> std::vector<T> {
  auto result = std::vector<T>();
  T tmp;
  while (std::cin >> tmp) {
    result.emplace_back(std::move(tmp));
  }
  return result;
}

/**
 * Calculate the duration of the trip between start_floor and end_floor.
 * The calculation is done with the assumption that the elevator moves at a
 * constant speed of 1m/s, and that all floors are at the bottom of the floor.
 */
template <typename HeightProperty>
auto floor_to_floor_trip(HeightProperty const& heights,
                         unsigned start_floor,
                         unsigned end_floor) -> unsigned {
  if (start_floor > end_floor) {
    std::swap(start_floor, end_floor);
  }
  unsigned total_time = 0;
  for (; start_floor != end_floor; ++start_floor) {
    total_time += heights[start_floor];
  }
  return total_time;
}

/**
 * Calculate the duration of the trip given with first and last.
 *
 * The trip is the sum of the durations of all stops, plus the trip lengths,
 * plus the duration of the last stop.
 */
template <typename StopProperty,
          typename HeightProperty,
          typename ForwardIterator>
unsigned trip_length(StopProperty const& stops,
                     HeightProperty const& heights,
                     ForwardIterator first,
                     ForwardIterator const last) {
  unsigned total_time = 0;
  auto const last_floor_iterator_pair = for_each_consecutive_pair(
      first,
      last,
      [&](auto const start_floor, auto const end_floor) {
        total_time += stops[start_floor] +
                      floor_to_floor_trip(heights, start_floor, end_floor);
      });
  auto const last_floor_iterator = last_floor_iterator_pair.first;
  total_time += stops[*last_floor_iterator];
  return total_time;
}

/**
 * A property that is constant for all indices.
 */
template <typename T>
struct const_property {
  T value;
  T operator[](size_t) const { return value; }
};

auto read_property() {
  std::string type;
  std::cin >> type;
  if (type == "array") {
    size_t length;
    std::cin >> length;
    return unsigned_property{read_n<unsigned>(length)};
  } else if (type == "constant") {
    unsigned constant;
    std::cin >> constant;
    return unsigned_property{const_property<unsigned>{constant}};
  } else {
    std::cerr << "Property type ('" << type << "') not recognised.\n";
    std::exit(1);
  }
}

int main() {
  auto const stop_length = read_property();
  auto const floor_height = read_property();
  auto const floor_sequence = read_till_end<unsigned>();

  // calculate and output
  std::cout << trip_length(stop_length,
                           floor_height,
                           floor_sequence.cbegin(),
                           floor_sequence.cend()) << '\n';
  return 0;
}
