#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>

#include "config.h"
#include "queue.h"
#include "globals.h"
#include "logger.h"

volatile sig_atomic_t stop = 0;

void handle_sigint(int signum) {
    (void)signum;
    stop = 1;
}

void install_sigint_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    // no SA_RESTART, so accept() is interrupted by SIGINT
    sigaction(SIGINT, &sa, NULL);
}

// Updated to accept thread count
void threadpool_start(int num_threads);
void threadpool_stop();

int main(int argc, char *argv[]) {
    install_sigint_handler();

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("Failed to ignore SIGPIPE");
        exit(EXIT_FAILURE);
    }

    // 1) Load config (flags or config file)
    server_config_t cfg;
    if (load_config(argc, argv, &cfg) != 0) {
        fprintf(stderr, "Error loading configuration\n");
        exit(EXIT_FAILURE);
    }
    // Set the global root for all workers to use:
    SERVER_ROOT = cfg.root_dir;

    // 2) Initialize subsystems
    logger_init("access.log", "error.log");
    queue_init(16);

    // 3) Start worker threads
    threadpool_start(cfg.thread_count);

    // 4) Set up listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(cfg.port)
    };

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf(
        "Threaded server listening on port %d\n"
        "  Document root: %s\n"
        "  Worker threads: %d\n",
        cfg.port, cfg.root_dir, cfg.thread_count
    );

    // 5) Accept loop
    socklen_t addrlen = sizeof(addr);
    while (!stop) {
        int client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR && stop) {
                break;  // SIGINT, shutdown
            }
            perror("accept");
            continue;
        }
        queue_push(client_fd);
    }

    // 6) Shutdown
    printf("Shutting down server...\n");
    threadpool_stop();
    logger_close();
    close(server_fd);
    queue_destroy();
    free(cfg.root_dir);
    printf("Server shutdown complete.\n");
    return 0;
}
