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

// #include "boost/core/demangle.hpp"

#include <typeinfo>
#include <string>
#include <vector>
#include <utility>

namespace dbg_util {
std::string replace_all(std::string in_what,
                        std::string const& what,
                        std::string const& with) {
  auto where = in_what.find(what);
  while (where != std::string::npos) {
    in_what.replace(where, what.size(), with);
    where = in_what.find(what, where);
  }
  return in_what;
}

/**
 * A demangling routine that also replaces some known standard types with their
 * commonly-known aliases to make them shorter.
 *
 * Currently handled:
 * - std::string
 * 
 * Also compresses old-style template endings with spaces between the >'s with
 * no spaces.
 */
std::string demangle(char const* const name) {
  using std::string; using std::vector; using std::make_pair; using std::pair;
  using std::move;
  string demangled = name;
  static vector<pair<string, string>> const replacements{
      {"std::__1::basic_string<char, std::__1::char_traits<char>, "
       "std::__1::allocator<char> >",
       "std::string"},
      {" >", ">"}};
  for (auto const& replacement : replacements) {
    demangled =
        replace_all(move(demangled), replacement.first, replacement.second);
  }
  return demangled;
}
}
