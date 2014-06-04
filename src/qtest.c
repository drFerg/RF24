#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  int nums[100];
  int i;
  int *p;
  Queue *q = q_create(10);
  for (i = 0; i < 100; i++){
    nums[i] = i;
    printf("%d\n", nums[i]);
    q_add(q, nums + i);
    p = q_remove(q);
    if (p) printf("%d\n", *p);
  }

  return 0;
}