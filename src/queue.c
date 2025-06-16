#include <pthread.h>
#include <stdlib.h>

static int *buffer;
static int capacity, size, front, rear;
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
    while (size == 0)
        pthread_cond_wait(&not_empty, &lock);
    int fd = buffer[front];
    front = (front + 1) % capacity;
    size--;
    pthread_mutex_unlock(&lock);
    return fd;
}

