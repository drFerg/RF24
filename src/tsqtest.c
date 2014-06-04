#include "tsqueue.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  int nums[100];
  int i;
  int *p;
  TSQueue *q = tsq_create(10);
  for (i = 0; i < 100; i++){
    nums[i] = i;
    printf("%d\n", nums[i]);
    tsq_add(q, nums + i, 1);
    p = tsq_remove(q, 1);
    if (p) printf("%d\n", *p);
  }
  tsq_destroy(q);
  return 0;
}