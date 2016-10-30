#include "ptrargs.h"

#ifdef USE_EXT
int i = 123;
#endif

extern SS   ss_e;

int main()
{
  report(ss_e);
  report1();
  return 0;
}


