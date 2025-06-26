#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_PORT      8080
#define DEFAULT_ROOT_DIR  "www/"
#define DEFAULT_THREADS   4

typedef struct {
    int port;
    char *root_dir;
    int thread_count;
} server_config_t;

// Parse CLI flags; if config_file is non-NULL and flags aren’t provided, read from it.
// Returns 0 on success, -1 on failure.
int load_config(int argc, char *argv[], server_config_t *cfg);

#endif // CONFIG_H
