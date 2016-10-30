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

#include <string>
#include <cassert>
#include <vector>
#include <iostream>
#include <deque>
#include <stdexcept>
#include <system_error>
#include "boost/variant.hpp"

//-- START UNION
union int_or_string {
  enum type {
    NOTHING,
    INT,
    STRING
  };
  /** put the t member always in the same place */
  template <type T, typename ValueType>
  struct tagged_value_t {
    type t;
    ValueType value;
    tagged_value_t() : t{T}, value{} {}
    tagged_value_t(ValueType x) : t{T}, value{x} {}
    tagged_value_t(tagged_value_t&&) = default; // have correct type already
    tagged_value_t(tagged_value_t const&) = default; // have correct type already
    tagged_value_t& operator=(tagged_value_t const& x) {
      t = T;
      value = x.value;
      return *this;
    }
    tagged_value_t& operator=(tagged_value_t&& x) {
      t = T;
      value = std::move(x.value);
      return *this;
    }
    ~tagged_value_t() = default;
    friend bool operator==(tagged_value_t const& x, tagged_value_t const& y) {
      return x.value == y.value;
    }
    friend bool operator<(tagged_value_t const& x, tagged_value_t const& y) {
      return x.value < y.value;
    }
  };

  // possible types
  tagged_value_t<NOTHING, char> nothing;
  tagged_value_t<INT, int> integer;
  tagged_value_t<STRING, std::string> string;

  // type accessor
  type t() const { return nothing.t; }

  // int support
  int_or_string(int x) : integer{std::move(x)} {}
  int_or_string& operator=(int x) {
    new (&integer) decltype(integer)(std::move(x));
    return *this;
  }
  // string support
  int_or_string(std::string x) : string{std::move(x)} {}
  int_or_string& operator=(std::string x) {
    new (&string) decltype(string)(std::move(x));
    return *this;
  }

  // semiregular
  int_or_string() {}
  ~int_or_string() {
    switch (t()) {
    case NOTHING:
      using nothing_t = decltype(nothing);
      nothing.~nothing_t();
      break;
    case INT:
      using integer_t = decltype(integer);
      integer.~integer_t();
      break;
    case STRING:
      using string_t = decltype(string);
      string.~string_t();
      break;
    }
  }
  int_or_string(int_or_string const& x) {
    switch (x.t()) {
    case NOTHING:
      new (&nothing) decltype(nothing)(x.nothing);
      break;
    case INT:
      new (&integer) decltype(integer)(x.integer);
      break;
    case STRING:
      new (&string) decltype(string)(x.string);
      break;
    }
  }
  int_or_string(int_or_string&& x) {
    switch (x.t()) {
    case NOTHING:
      new (&nothing) decltype(nothing)(std::move(x.nothing));
      break;
    case INT:
      new (&integer) decltype(integer)(std::move(x.integer));
      break;
    case STRING:
      new (&string) decltype(string)(std::move(x.string));
      break;
    }
  }
  int_or_string& operator=(int_or_string const& x) {
    if (t() == x.t()) {
      switch (x.t()) {
      case NOTHING:
        nothing = x.nothing;
        break;
      case INT:
        integer = x.integer;
        break;
      case STRING:
        string = x.string;
        break;
      }
    } else {
      this->~int_or_string(); // destroy the value
      new (this) int_or_string(x); // dispatch to constructor
    }
    return *this;
  }
  int_or_string& operator=(int_or_string&& x) {
    if (t() == x.t()) {
      switch (x.t()) {
      case NOTHING:
        nothing = std::move(x.nothing);
        break;
      case INT:
        integer = std::move(x.integer);
        break;
      case STRING:
        string = std::move(x.string);
        break;
      }
    } else {
      this->~int_or_string();      // destroy the value
      new (this) int_or_string(std::move(x));  // dispatch to constructor
    }
    return *this;
  }

  // equality
  friend bool operator==(int_or_string const& x, int_or_string const& y) {
    if (x.t() == y.t()) {
      switch (x.t()) {
      case int_or_string::type::NOTHING:
        return x.nothing == y.nothing;
      case int_or_string::type::INT:
        return x.integer == y.integer;
      case int_or_string::type::STRING:
        return x.string == y.string;
      }
    } else {
      return false;
    }
  }
  friend bool operator!=(int_or_string const& x, int_or_string const& y) {
    return !(x == y);
  }

  // total order
  friend bool operator<(int_or_string const& x, int_or_string const& y) {
    if (x.t() == y.t()) {
      switch (x.t()) {
      case int_or_string::type::NOTHING:
        return x.nothing < y.nothing;
      case int_or_string::type::INT:
        return x.integer < y.integer;
      case int_or_string::type::STRING:
        return x.string < y.string;
      }
    } else {
      return x.t() < y.t();
    }
  }
};

//-- END UNION

int main() {
  int_or_string x;         // default construction
  int_or_string y{5};      // construction from int
  int_or_string z{"foo!"}; // construction from string
  auto w(x);               // construction from int_or_string
  x = "foo!";              // assignment from string
  assert(x == z);          // equality operator between strings
  assert(!(x != z));       // inequality is defined correctly
  assert(x != y);          // different types are not equal
  x = 5;                   // assignment from int
  assert(x == y);          // equality operator between ints
  y = 4;
  assert(y < x);           // less-than-comparable between ints
  w = z;
  assert(!(w < z));        // less-than-comparable between strings
  assert(x < z);           // less-than-comparable between int and string
  assert(!(z < x));        // assert less-than-comparable works correctly
  w = "foo";
  auto a = std::move(w);   // initialize from move
  // assert move, not copy
  assert(w.string.value != std::string("foo"));
  int_or_string b = std::string("bar");
  a = std::move(b);        // move assignment
  // assert move, not copy
  assert(b.string.value != std::string("bar")); // make sure it got moved, not copied
}




#if 0
// pseudo-haskell
data Maybe a = Just a | Nothing;

data Errors = IOError | LogicError;
data Possibly a = Result a | Error Errors;

foo :: Maybe Int -> Possibly Int
foo Just value = Result value
foo Nothing = Error LogicError
#endif


struct nothing {};
template <typename T>
using Maybe = boost::variant<T, nothing>;

using maybe_int = Maybe<int>;

template <typename T>
struct Result { T value; };

using Errors = boost::variant<std::logic_error, std::system_error>;
template <typename T>
using Possibly = boost::variant<Result<T>, Errors>;

struct foo : boost::static_visitor<Possibly<int>> {
  using return_type = Possibly<int>;

  int other_parameter;

  foo(int x) : other_parameter{x} {}

  return_type operator()(int x) const { return Result<int>{x+other_parameter}; }
  return_type operator()(nothing) const { return Errors{std::logic_error{""}}; }
};

void test() {
  auto const w = Maybe<int>{0};
  boost::apply_visitor(foo{5}, w);
  auto x = Maybe<int>{0};
  auto& my_int_ref = boost::get<int>(x);
  auto* my_int_ptr = boost::get<int>(&x);
}

using boo = boost::variant<std::string, std::vector<char>, std::deque<char>, int>;


struct print : boost::static_visitor<void> {
  template <typename T>
  std::enable_if_t<!std::is_same<T, std::string>{}, void>
  operator()(T const& x) const {
    std::copy(x.begin(), x.end(), std::ostream_iterator<char>(std::cout));
  }
  template <typename T>
  void operator()(T const& x) const {
    std::cout << x;
  }
};

