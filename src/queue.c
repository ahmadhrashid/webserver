#include "queue.h"
#include <pthread.h>
#include <stdlib.h>

static int *buffer, capacity, size, front, rear;
static int shutdown_flag = 0;   
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void queue_init(int cap) {
    capacity = cap;
    size = front = rear = 0;
    buffer = malloc(sizeof(int) * cap);
}

void queue_destroy() {
    free(buffer);
}

void queue_shutdown() {
    printf("[queue] shutdown_flag=1, waking all workers\n");
    pthread_mutex_lock(&lock);
    shutdown_flag = 1;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&lock);
}

void queue_push(int client_fd) {
    pthread_mutex_lock(&lock);
    buffer[rear] = client_fd;
    rear = (rear + 1) % capacity;
    size++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
}

int queue_pop() {
    pthread_mutex_lock(&lock);

    // Wait until there’s something in the queue OR we’ve been told to shut down
    while (size == 0 && !shutdown_flag) {
        pthread_cond_wait(&not_empty, &lock);
    }

    // If we’re shutting down, bail out
    if (shutdown_flag) {
        pthread_mutex_unlock(&lock);
        return -1;
    }

    // Otherwise pop the next FD
    int fd = buffer[front];
    front = (front + 1) % capacity;
    size--;
    pthread_mutex_unlock(&lock);
    return fd;
}
