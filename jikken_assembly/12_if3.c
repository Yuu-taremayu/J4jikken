#include "h8-3052-iodef.h"

int main(void)
{
  int i;
  
  i = 0;

  /* 自作ver. */
  if (i == 10) {
    i = 0;
  }
  else {
    i = 10;
  }

  /* 解答ver. */
  /*
  if (i == 10)
  {
    i = 0;
  }
  else
  {
    i = 10;
  }
  */

  return 0;
}
