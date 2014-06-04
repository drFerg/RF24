#ifndef TSQUEUE_H
#define TSQUEUE_H

typedef struct tsqueue TSQueue;

TSQueue *tsq_create(int size);
int tsq_add(TSQueue *q, void *element, int blocking);
void *tsq_remove(TSQueue *q, int blocking);
int tsq_count(TSQueue *q);
void tsq_destroy(TSQueue *q);

#endif /* TSQUEUE_H */