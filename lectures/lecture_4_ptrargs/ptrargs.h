#ifndef PTRARGS_H
#define PTRARGS_H

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

#endif
