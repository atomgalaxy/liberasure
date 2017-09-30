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

#include <iostream>

#define REPORT(x) \
  std::cout << __FILE__ ", " << __PRETTY_FUNCTION__ \
  << ":" << __LINE__ << ": " #x " = " << (x) << std::endl

// #define USE_EXT
#define USE_FUNC

template<typename T, T * p>
struct S {
#ifndef USE_FUNC
  static constexpr T * ptr = p;
#else
  static constexpr T * ptr() { return p; }
#endif
};

#ifdef USE_EXT
extern int i;
#else
static int i;
#endif

using SS = S<int, &i>;

template<typename T>
void
report(const T & t)
{
#ifndef USE_FUNC
  auto p = t.ptr;
#else
  auto p = t.ptr();
#endif
  REPORT(p);
  REPORT(*p);
}

void
report1();
