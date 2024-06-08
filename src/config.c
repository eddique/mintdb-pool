#include "config.h"
#include <stdlib.h>
#include <string.h>
Config *config = NULL;

// void load_dotenv()
// {
//     FILE *file = fopen(".env", "r");
//     if (file == NULL)
//     {
//         perror("Error opening .env file");
//         return;
//     }

//     char line[256];
//     while (fgets(line, sizeof(line), file) != NULL)
//     {
//         char *name = strtok(line, "=");
//         char *value = strtok(NULL, "\n");

//         if (name != NULL && value != NULL)
//         {
//             setenv(name, value, 1);
//         }
//     }

//     fclose(file);
// }

void init_config()
{
    config = (Config *)malloc(sizeof(Config));
    char *target_host = getenv("HOST_PREFIX");
    char *target_port_str = getenv("TARGET_PORT");
    char *replicas_str = getenv("TARGET_PORT");
    if (target_host != NULL)
    {
        strncpy(config->target_host_prefix, target_host, MAX_CHAR - 1);
        config->target_host_prefix[MAX_CHAR - 1] = '\0';
    }
    else
    {
        strcpy(config->target_host_prefix, "mintdb");
    }
    if (target_port_str != NULL)
    {
        config->target_port = atoi(target_port_str);
    }
    else
    {
        config->target_port = 3000;
    }
    if (replicas_str != NULL)
    {
        config->replicas = atoi(replicas_str);
    }
    else
    {
        config->replicas = 3;
    }
}

void config_cleanup()
{
    free(config);
}