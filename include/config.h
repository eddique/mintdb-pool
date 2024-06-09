#ifndef mintdb_pool_config_h
#define MAX_CHAR 255
typedef struct {
    int replicas;
    char target_host_prefix[MAX_CHAR];
    int target_port;
    int port;
} Config;
Config *config;
void init_config();
void config_cleanup();
#endif