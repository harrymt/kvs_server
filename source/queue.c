#include "queue.h"
#include "safe_functions.h"

/**
 * Helper function to init a semaphore.
 */
sem_t *make_semaphore(int value) {
	sem_t *sem = malloc_safe(sizeof(sem_t));
	if (sem_init(sem, 0, value) != 0) perror_exit("sem_init failed");
	return sem;
}

/**
 * Setup and make a queue.
 */
Queue *make_queue(int length)
{
  Queue *queue = (Queue *) malloc_safe(sizeof(Queue));
  if(queue == NULL) perror_exit("Queue malloc failed");
  queue->length = length;
  queue->items = malloc_safe(length * sizeof(queue_item));
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
	sem_wait_safe(queue->elements);
	sem_wait_safe(queue->mutex);

	queue_item item = queue->items[queue->next_out];
	queue->next_out = (queue->next_out + 1) % queue->length;

	sem_post_safe(queue->mutex);
	sem_post_safe(queue->spaces);

	return item;
}

/**
 * Push an item onto the queue.
 */
void queue_push(Queue *queue, queue_item item) {
	sem_wait_safe(queue->spaces);
	sem_wait_safe(queue->mutex);

	queue->items[queue->next_in] = item;
	queue->next_in = (queue->next_in + 1) % queue->length;

	sem_post_safe(queue->mutex);
	sem_post_safe(queue->elements);
}
