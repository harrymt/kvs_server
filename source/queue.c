#include "queue.h"

/**
 * Helper function to init a semaphore.
 */
sem_t *make_semaphore(int value) {
	sem_t *sem = malloc(sizeof(sem_t));
	if(sem == NULL) perror_line("Sem malloc failed");
	if (sem_init(sem, 0, value) != 0) perror_line("sem_init failed");
	return sem;
}

/**
 * Setup and make a queue.
 */
Queue *make_queue(int length)
{
  Queue *queue = (Queue *) malloc(sizeof(Queue));
  if(queue == NULL) perror_line("Queue malloc failed");
  queue->length = length;
  queue->items = malloc(length * sizeof(queue_item));
  queue->next_in = 0;
  queue->next_out = 0;
  queue->mutex = make_semaphore(1);
  queue->elements = make_semaphore(0);
  queue->spaces = make_semaphore(length - 1);
  return queue;
}


/**
 * Pop an item off a queue.
 */
queue_item queue_pop(Queue *queue) {
	sem_wait(queue->elements);
	sem_wait(queue->mutex);

	queue_item item = queue->items[queue->next_out];
	queue->next_out = (queue->next_out + 1) % queue->length; // wrap around

	sem_post(queue->mutex);
	sem_post(queue->spaces);

	return item;
}

/**
 * Push an item onto the quque.
 */
void queue_push(Queue *queue, queue_item item) {
	sem_wait(queue->spaces);
	sem_wait(queue->mutex);

	queue->items[queue->next_in] = item;
	queue->next_in = (queue->next_in + 1) % queue->length; // wrap around

	sem_post(queue->mutex);
	sem_post(queue->elements);
}
