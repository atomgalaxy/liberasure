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

#include <memory>
#include <ostream>
#include <string>

struct unique_string {
  std::unique_ptr<std::string> value;
  friend auto operator==(unique_string const &x, unique_string const &y)
      -> bool {
    return x.value == y.value;
  }
};
auto operator<<(std::ostream &o, unique_string const &x) -> std::ostream & {
  return o << (x.value ? *x.value : std::string("nullptr"));
}
