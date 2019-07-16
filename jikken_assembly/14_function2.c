#include "h8-3052-iodef.h"

int func1(void)
{
  int i, j, k;

  j = func2();

  return j;
}

int func2(void)
{
  return 5;
}

int main(void)
{
  int i;

  i = func1();

  return 0;
}
