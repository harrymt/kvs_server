
#ifndef _queue_h_
#define _queue_h_

#include "server.h"
#include <semaphore.h>

typedef struct queue_item
{
	int port;
	int sock;
	int type;
} queue_item;

// FILO queue
typedef struct {
  queue_item *items;
  int length;
  int next_in;
  int next_out;
  sem_t *mutex;
  sem_t *elements;
  sem_t *spaces;
} Queue;

sem_t *make_semaphore(int value);

Queue *make_queue(int length);

queue_item queue_pop(Queue *queue);

void queue_push(Queue *queue, queue_item item);

#endif

