#ifndef QUEUE_H
#define QUEUE_H

typedef struct queue Queue;

Queue *q_create(int size);
int q_add(Queue *q, void *element);
void *q_remove(Queue *q);
int q_count(Queue *q);
int q_size(Queue *q);
void q_destroy(Queue *q);

#endif /* QUEUE_H */