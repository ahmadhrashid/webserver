#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include "queue.h"

void threadpool_start(); // from threadpool.c

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    queue_init(16);
    threadpool_start();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 10);

    printf("Threaded server listening on port 8080...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        queue_push(client_fd);
    }

    close(server_fd);
    queue_destroy();
    return 0;
}

