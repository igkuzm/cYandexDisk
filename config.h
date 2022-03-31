/**
 * File              : config.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 11.09.2021
 * Last Modified Date: 30.03.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include <stdio.h>

#define CONFIG_SIZE (256)
#define CLIENT_ID_SET (1)
#define CLIENT_SECRET_SET (2)
#define DEVICE_ID_SET (4)
#define DEVICE_NAME_SET (8)
#define TOKEN_SET (16)

typedef struct config {
	unsigned set;
	char client_id[128];
	char client_secret[128];
	char device_id[37];
	char device_name[101];
	char token[64];
} CONFIG;

//init config
void config_init(CONFIG *config);

//write config to file
int config_write(CONFIG *config, const char *file);

//read config from file - return config->set
unsigned config_read(CONFIG *config, const char *file);

//differ config 
unsigned config_diff(CONFIG *oldconfig, CONFIG *newconfig);

#define config_set(config, key, value) ({strncpy(config->key, value, sizeof(config->key)-1); config->key[sizeof(config->key)-1] = '\0';})
