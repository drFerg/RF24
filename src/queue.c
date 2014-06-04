#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

typedef struct queue {
  int size;
  int count;
  void **elements;
  int in;
  int out;
} Queue;

Queue *q_create(int size) {
  Queue *q = (Queue *)malloc(sizeof(Queue));
  if (q == NULL) return NULL;
  q->size = size;
  q->count = 0;
  q->in = 0;
  q->out = 0;
  q->elements = (void **)malloc(size * sizeof(void *));
  if (q->elements == NULL) {
    free(q);
    return NULL;
  } 
  return q;
}

int q_add(Queue *q, void *element) {
  if (q->count == q->size) return 0;
  q->elements[q->in] = element;
  q->in = (q->in + 1) % q->size;
  q->count++;
  return 1;
}

void *q_remove(Queue *q) {
  if (q->count == 0) return NULL;
  void * element = q->elements[q->out];
  q->out = (q->out + 1) % q->size;
  q->count--;
  return element;
}

int q_count(Queue *q) {
  return q->count;
}

int q_size(Queue *q) {
  return q->size;
}

void q_destroy(Queue *q) {
  free(q->elements);
  free(q);
}