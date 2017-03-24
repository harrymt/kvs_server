#include "safe_functions.h"

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "server.h"
#include "debug.h"

void pthread_cond_wait_safe(pthread_cond_t *cond, pthread_mutex_t *m) {
	 int result = pthread_cond_wait(cond, m);
	 if(result != 0) {
		 perror_exit("Error waiting on condition variable.");
	 }
}
void pthread_cond_signal_safe(pthread_cond_t *m) {
	int result = pthread_cond_signal(m);
	if(result != 0) {
		perror_exit("Error signalling condition variable.");
	}
}

void pthread_join_safe(pthread_t t) {
	int result = pthread_join(t, NULL);
	if(result != 0) {
		printf("(45: deadlock) (4: no such process) result %d\n", result);
		perror_exit("Error joining thread.");
	}
}

void close_safe(int s) {
	int result = close(s);
	if(result != 0) {
		perror_exit("Error closing socket!");
	}
}

void sem_post_safe(sem_t *sem) {
	int result = sem_post(sem);
	if(result != 0) {
		perror_exit("Error posting semaphore.");
	}
}

void sem_wait_safe(sem_t *sem) {
	int result = sem_wait(sem);
	if(result != 0) {
		perror_exit("Error waiting on semaphore.");
	}
}

void pthread_mutex_lock_safe(pthread_mutex_t *m) {
	int result = pthread_mutex_lock(m);
	if(result != 0) {
		perror_exit("Error locking mutex");
	}
}

void pthread_mutex_unlock_safe(pthread_mutex_t *m) {
	int result = pthread_mutex_unlock(m);
	if(result != 0) {
		perror_exit("Error unlocking mutex");
	}
}

/**
 * Performs a safe version of malloc.
 */
void* malloc_safe(size_t size) {
	void* result = malloc(size);
	if(result == NULL) {
		perror_exit("Error in malloc.");
	}
	return result;
}

/**
 * Safe versions of sprintf.
 */
void sprintf_safe(char* message, const char* text) {
	 if(sprintf(message, text) < 0) {
		perror_exit("Error with sprintf.");
	 }
}
void sprintf_arg_safe(char* message, const char* text, int arg) {
	 if(sprintf(message, text, arg) < 0) {
		perror_exit("Error with sprintf.");
	 }
}
void sprintf_arg_str_safe(char* message, const char* text, const char* arg) {
	 if(sprintf(message, text, arg) < 0) {
		perror_exit("Error with sprintf.");
	 }
}

