#include <pthread.h>
#include <stdlib.h>
#include "queue.h"
#include "tsqueue.h"

typedef struct tsqueue {
  Queue *q;
  pthread_cond_t cond;
  pthread_mutex_t lock;
} TSQueue;

TSQueue *tsq_create(int size) {
  TSQueue *q = (TSQueue *)malloc(sizeof(TSQueue));
  if (q == NULL) return NULL;
  q->q = q_create(size);
  if (q->q == NULL) return NULL;
  pthread_mutex_init(&(q->lock), NULL);
  pthread_cond_init(&(q->cond), NULL);
  return q;
}

int tsq_add(TSQueue *q, void *element, int blocking) {
  int result;
  pthread_mutex_lock(&(q->lock));
  while (blocking && (q_count(q->q) == q_size(q->q)))
    pthread_cond_wait(&(q->cond), &(q->lock));
  result = q_add(q->q, element);
  if (result) pthread_cond_signal(&(q->cond));
  pthread_mutex_unlock(&(q->lock));
  return result;
}

void *tsq_remove(TSQueue *q, int blocking) {
  void * element;
  pthread_mutex_lock(&(q->lock));
  while (blocking && (q_count(q->q) == 0))
    pthread_cond_wait(&(q->cond), &(q->lock));
  element = q_remove(q->q);
  if (element) pthread_cond_signal(&(q->cond));
  pthread_mutex_unlock(&(q->lock));
  return element;
}

int tsq_count(TSQueue *q) {
  int count;
  pthread_mutex_lock(&(q->lock));
  count = q_count(q->q);
  pthread_mutex_unlock(&(q->lock));
  return count;
}

void tsq_destroy(TSQueue *q) {
  pthread_mutex_lock(&(q->lock));
  q_destroy(q->q);
  pthread_mutex_unlock(&(q->lock));
  pthread_mutex_destroy(&(q->lock));
  pthread_cond_destroy(&(q->cond));
  free(q);
}