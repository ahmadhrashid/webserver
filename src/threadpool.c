#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "queue.h"

#define THREAD_COUNT 4

static void* worker(void *arg) {
    while (1) {
        int client_fd = queue_pop();

        char buffer[1024];
        read(client_fd, buffer, sizeof(buffer));
        printf("Request:\n%s\n", buffer);

        const char *response =
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<h1>Hello from your threaded server!</h1>";

        write(client_fd, response, strlen(response));
        close(client_fd);
    }
    return NULL;
}

void threadpool_start() {
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&threads[i], NULL, worker, NULL);
        pthread_detach(threads[i]);
    }
}

