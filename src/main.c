#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include "queue.h"
#include <signal.h>

volatile sig_atomic_t stop = 0;

void handle_sigint(int signum) {
    stop = 1;
}

void threadpool_start(); // from threadpool.c

int main() {
    signal(SIGINT, handle_sigint);
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("Failed to ignore SIGPIPE");
        exit(EXIT_FAILURE);
    }
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

    while (!stop) {
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            if (stop) break; // Exit loop if server is stopping
            perror("Failed to accept connection");
            continue;
        }
        queue_push(client_fd);
        printf("Accepted connection from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    }
    printf("Shutting down server...\n");
    fflush(stdout);
    printf("Server shutdown complete.\n");
     

    close(server_fd);
    queue_destroy();
    return 0;
}

