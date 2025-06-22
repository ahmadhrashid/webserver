#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "queue.h"
#include <signal.h>
#include "globals.h"
#include "logger.h"

volatile sig_atomic_t stop = 0;

void handle_sigint(int signum)
{
    (void)signum; // Unused parameter
    stop = 1;
}
void install_sigint_handler()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    // no SA_RESTART, so accept() is interrupted
    sigaction(SIGINT, &sa, NULL);
}
void threadpool_start(); // from threadpool.c
void threadpool_stop();

int main()
{
    install_sigint_handler();
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        perror("Failed to ignore SIGPIPE");
        exit(EXIT_FAILURE);
    }
    logger_init("access.log", "error.log");
    queue_init(16);
    threadpool_start();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {.sin_family = AF_INET,
                               .sin_addr.s_addr = INADDR_ANY,
                               .sin_port = htons(8080)};
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Threaded server listening on port 8080...\n");

    socklen_t addrlen = sizeof(addr);
    while (!stop)
    {
        int client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0)
        {
            if (errno == EINTR && stop)
                break; // interrupted by SIGINT
            perror("accept");
            continue;
        }
        queue_push(client_fd);
    }

    printf("Shutting down server...\n");
    // stop threads & clean up
    threadpool_stop();
    logger_close();
    close(server_fd);
    queue_destroy();
    printf("Server shutdown complete.\n");
    return 0;
}
