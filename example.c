#include <stdio.h>

void cleanup(void) {
  printf("cleanup\n");
  fflush(stdout);
}

void hello_world_func(void) {
  printf("hello world!\n");
  fflush(stdout);
}
