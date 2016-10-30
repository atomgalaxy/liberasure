#pragma once

#include <sstream>
#include <ostream>
#include <string>

template <typename Tag>
struct atom {
  friend bool operator==(atom const&, atom const&) { return true; }
  friend bool operator<(atom const&, atom const&) { return false; }
  friend std::ostream& operator<<(std::ostream& o, atom const& x) {
    return o << typeid(x).name();
  }
};
