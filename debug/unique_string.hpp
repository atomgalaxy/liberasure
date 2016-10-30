#pragma once

#include <string>
#include <ostream>
#include <memory>

struct unique_string {
  std::unique_ptr<std::string> value;
  friend bool operator==(unique_string const& x, unique_string const& y) {
    return x.value == y.value;
  }
};
std::ostream& operator<<(std::ostream& o, unique_string const& x) {
  return o << (x.value ? *x.value : std::string("nullptr"));
}
