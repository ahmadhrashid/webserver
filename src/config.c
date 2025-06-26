// src/config.c
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "  -p, --port <port>        Listening port (default %d)\n"
        "  -r, --root <directory>   Document root (default \"%s\")\n"
        "  -t, --threads <count>    Number of worker threads (default %d)\n"
        "  -c, --config <file>      Config file (optional)\n"
        "  -h, --help               Show this help message\n",
        prog, DEFAULT_PORT, DEFAULT_ROOT_DIR, DEFAULT_THREADS);
}

int load_config(int argc, char *argv[], server_config_t *cfg) {
    // 1) Set defaults
    cfg->port         = DEFAULT_PORT;
    cfg->root_dir     = strdup(DEFAULT_ROOT_DIR);
    cfg->thread_count = DEFAULT_THREADS;

    int port_set    = 0;
    int root_set    = 0;
    int threads_set = 0;
    char *conf_path = NULL;

    // 2) Single getopt_long loop
    static struct option opts[] = {
        {"port",    required_argument, NULL, 'p'},
        {"root",    required_argument, NULL, 'r'},
        {"threads", required_argument, NULL, 't'},
        {"config",  required_argument, NULL, 'c'},
        {"help",    no_argument,       NULL, 'h'},
        {0,0,0,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "p:r:t:c:h", opts, NULL)) != -1) {
        switch (opt) {
            case 'p':
                cfg->port = atoi(optarg);
                port_set = 1;
                break;
            case 'r':
                free(cfg->root_dir);
                cfg->root_dir = strdup(optarg);
                root_set = 1;
                break;
            case 't':
                cfg->thread_count = atoi(optarg);
                threads_set = 1;
                break;
            case 'c':
                conf_path = strdup(optarg);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                return -1;
        }
    }

    // 3) If a config file was given, parse it now (filling only unset values)
    if (conf_path) {
        FILE *f = fopen(conf_path, "r");
        if (!f) {
            perror("Opening config file");
            free(conf_path);
            return -1;
        }
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char key[64], val[192];
            if (sscanf(line, "%63[^=]=%191[^\n]", key, val) == 2) {
                if (!port_set && strcmp(key, "port") == 0) {
                    cfg->port = atoi(val);
                } else if (!root_set && strcmp(key, "root") == 0) {
                    free(cfg->root_dir);
                    cfg->root_dir = strdup(val);
                } else if (!threads_set && strcmp(key, "threads") == 0) {
                    cfg->thread_count = atoi(val);
                }
            }
        }
        fclose(f);
        free(conf_path);
    }

    return 0;
}
