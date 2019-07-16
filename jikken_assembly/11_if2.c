#include "h8-3052-iodef.h"

int main(void)
{
  int i;

  i = 10;

  /* 自作ver. */
  if (i >= 0) {
    i = 255;
  }
  else {
    i = 0;
  }

  /* 解答ver. */
  /*
  if (i < 0)
  {
    i = 0;
  }
  else
  {
    i = 255;
  }
  */

  return 0;
}

