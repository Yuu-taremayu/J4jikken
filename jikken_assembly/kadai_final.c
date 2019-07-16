#include "h8-3052-iodef.h"

void sub_1(void)
{
  int i;

  i = 0;
}

int sub_2(void)
{
  return 0;
}

int sub_3(void)
{
  int i;

  i = 1;

  return i;
}

int sub_4(int i)
{
  int j;

  if (i == 0) {
    j = 1;
  }
  else if (i == 1) {
    j = 10;
  }
  else {
    j = 100;
  }

  return j;
}

int sub_5(int i)
{
  int j, k;

  k = 0;

  for (j = 0; j < i; j++) {
    k = k + j;
  }

  return k;
}

int main(void)
{
  int i;

  sub_1();
  sub_2();
  
  i = sub_3();

  i = sub_4(i);

  i = sub_5(i);

  return i;
}
