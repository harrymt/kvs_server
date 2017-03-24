
#ifndef _safe_functions_h_
#define _safe_functions_h_

#include <pthread.h>
#include <sys/socket.h>
#include <semaphore.h>
#include "server.h"

void* malloc_safe(size_t a);
void sprintf_safe(char* message, const char* text);
void sprintf_arg_safe(char* message, const char* text, int more);
void sprintf_arg_str_safe(char* message, const char* text, const char* arg);
void pthread_mutex_lock_safe(pthread_mutex_t *m);
void pthread_mutex_unlock_safe(pthread_mutex_t *m);
void sem_wait_safe(sem_t *sem);
void sem_post_safe(sem_t *sem);
void close_safe(int s);
void pthread_join_safe(pthread_t t);
void pthread_cond_signal_safe(pthread_cond_t *m);
void pthread_cond_wait_safe(pthread_cond_t *cond, pthread_mutex_t *m);
void pthread_cond_signal_safe(pthread_cond_t *m);

#endif
