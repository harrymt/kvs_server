#include "queue.h"

#include <sys/socket.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include "kv.h"
#include "parser.h"
#include "server.h"
#include "debug.h"
#include "protocol_manager.h"
#include "socket-helper.h"
#include "server_helpers.h"
#include "message_manager.h"



void *safe_malloc(int size)
{
  void *p = malloc (size);
  if (p == NULL) perror ("malloc failed");
  return p;
}

sem_t *make_semaphore(int value)
{
	sem_t *sem = safe_malloc(sizeof(sem_t));
	if (sem_init(sem, 0, value) != 0) perror("sem_init failed");
	return sem;
}

Queue *make_queue(int length)
{
  Queue *queue = (Queue *) safe_malloc(sizeof(Queue));
  queue->length = length;
  queue->items = malloc(length * sizeof(queue_item));
  queue->next_in = 0;
  queue->next_out = 0;
  queue->mutex = make_semaphore(1);
  queue->elements = make_semaphore(0);
  queue->spaces = make_semaphore(length - 1);
  return queue;
}


queue_item queue_pop(Queue *queue) {
	sem_wait(queue->elements);
	sem_wait(queue->mutex);

	queue_item item = queue->items[queue->next_out];
	queue->next_out = (queue->next_out + 1) % queue->length; // wrap around

	sem_post(queue->mutex);
	sem_post(queue->spaces);

	return item;
}

void queue_push(Queue *queue, queue_item item) {
	sem_wait(queue->spaces);
	sem_wait(queue->mutex);

	queue->items[queue->next_in] = item;
	queue->next_in = (queue->next_in + 1) % queue->length; // wrap around

	sem_post(queue->mutex);
	sem_post(queue->elements);
}
