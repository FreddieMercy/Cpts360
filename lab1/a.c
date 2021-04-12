/************* a.c file ********************/
#include <stdio.h>
#include <stdlib.h>

extern int B();

int A(int x, int y)
{
  int d,e,f;
  d=4; e=5; f=6;
  B(d,e);
}
