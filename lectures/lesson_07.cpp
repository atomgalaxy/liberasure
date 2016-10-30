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

// CRTP
//

#include <iostream>

template <typename Derived>
struct open_doorable {
  void open_doors() {
    auto const doors = static_cast<Derived const*>(this)->door_count();
    std::cout << "Opened " << doors << "!\n";
  }
};


struct car : open_doorable<car> {
  unsigned door_count() const { return 5; }
  void foo() {
    open_doors();
  }
};

// ----------------------------------------------------------

struct nothing {};
template <typename Derived, typename Base = nothing>
struct equality_comparable : Base {
  friend bool operator!=(equality_comparable const& x,
                         equality_comparable const& y) {
    return !(static_cast<Derived const&>(x) == static_cast<Derived const&>(y));
  }
};

template <typename Derived, typename Base = nothing>
struct less_than_comparable : Base {
  friend bool operator>=(less_than_comparable const& x,
                         less_than_comparable const& y) {
    return !(static_cast<Derived const&>(x) < static_cast<Derived const&>(y));
  }
  friend bool operator>(less_than_comparable const& x,
                         less_than_comparable const& y) {
    return static_cast<Derived const&>(y) < static_cast<Derived const&>(x);
  }
  friend bool operator<=(less_than_comparable const& x,
                         less_than_comparable const& y) {
    return !(static_cast<Derived const&>(x) > static_cast<Derived const&>(y));
  }
};

template <typename T>
struct value_
    : equality_comparable<value_<T>,
                          less_than_comparable<value_<T>>> {
  T value;

  value_(T x) : value{std::move(x)} {}

  friend bool operator<(value_ const& x, value_ const& y) {
    return x.value < y.value;
  }
  friend bool operator==(value_ const& x, value_ const& y) {
    return x.value == y.value;
  }
};


int main() {
  value_<int> x(5);
  value_<int> y(6);
  x == y;
  x != y;
  x > y;
}














