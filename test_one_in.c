#include "./standalone.c"

int main()
{
  for (int i = 0; i < 100; i++) {
      if (one_in(3)) printf("yep\n");
      else
      printf("nope\n" );
  }

  return(0);

}
