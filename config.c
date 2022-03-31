/**
 * File              : config.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 11.09.2021
 * Last Modified Date: 30.03.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Parse the buffer for config info. Return an error code or 0 for no error.
int config_parse(char *buf, CONFIG *config) {
	char dummy[CONFIG_SIZE];
	if (sscanf(buf, " %s", dummy) == EOF) return 0; // blank line
	if (sscanf(buf, " %[#]", dummy) == 1) return 0; // comment starts with #
	if (sscanf(buf, " %[[]", dummy) == 1) return 0; // description like [DESCRIPTION]

	//parse client_id
	if (sscanf(buf, " client_id = %s", config->client_id) == 1) {
		if (config->set & CLIENT_ID_SET) return CLIENT_ID_SET; // error; host already set
		config->set |= CLIENT_ID_SET;
		return 0;
	}

	//parse client_secret
	if (sscanf(buf, " client_secret = %s", config->client_secret) == 1) {
		if (config->set & CLIENT_SECRET_SET) return CLIENT_SECRET_SET; // error; port already set
		config->set |= CLIENT_SECRET_SET;
		return 0;
	}

	//parse device_id
	if (sscanf(buf, " device_id = %s", config->device_id) == 1) {
		if (config->set & DEVICE_ID_SET) return DEVICE_ID_SET; // error; user already set
		config->set |= DEVICE_ID_SET;
		return 0;
	}

	//parse device_name
	if (sscanf(buf, " device_name = %s", config->device_name) == 1) {
		if (config->set & DEVICE_NAME_SET) return DEVICE_NAME_SET; // error; user already set
		config->set |= DEVICE_NAME_SET;
		return 0;
	}

	//parse token
	if (sscanf(buf, " token = %s", config->token) == 1) {
		if (config->set & TOKEN_SET) return TOKEN_SET; // error; user already set
		config->set |= TOKEN_SET;
		return 0;
	}
	
	return -1; // syntax error
}

void config_init(CONFIG *config) {
	config->set = 0u;
}

unsigned config_diff(CONFIG *oldconfig, CONFIG *newconfig){
	unsigned set = 0u;

	if (strncmp (oldconfig->client_id, newconfig->client_id, CONFIG_SIZE)) {
		set |= CLIENT_ID_SET;
	}
	if (strncmp (oldconfig->client_secret, newconfig->client_secret, CONFIG_SIZE)) {
		set |= CLIENT_SECRET_SET;
	}
	if (strncmp (oldconfig->device_id, newconfig->device_id, CONFIG_SIZE)) {
		set |= DEVICE_ID_SET;
	}
	if (strncmp (oldconfig->device_name, newconfig->device_name, CONFIG_SIZE)) {
		set |= DEVICE_NAME_SET;
	}
	if (strncmp (oldconfig->token, newconfig->token, CONFIG_SIZE)) {
		set |= TOKEN_SET;
	}

	return set;
}

int config_write(CONFIG *config, const char *file){
	FILE *f = fopen(file, "w+");
	if (!f) {
		perror("open file");
		return ENOENT;
	}

	fputs("#cYandexDisk.conf - config file\n", f);
	fputs("#this config generated automaticaly\n", f);
	fputs("\n", f);
	fputs("[cYandexDisk]\n", f);

	char client_id[CONFIG_SIZE];
	sprintf(client_id, "client_id = %s\n", config->client_id);
	fputs(client_id, f);

	char client_secret[CONFIG_SIZE];
	sprintf(client_secret, "client_secret = %s\n", config->client_secret);
	fputs(client_secret, f);

	char device_id[CONFIG_SIZE];  
	sprintf(device_id, "device_id = %s\n", config->device_id);
	fputs(device_id, f);

	char device_name[CONFIG_SIZE];  
	sprintf(device_name, "device_name = %s\n", config->device_name);
	fputs(device_name, f);

	char token[CONFIG_SIZE];    
	sprintf(token, "token = %s\n", config->token);
	fputs(token, f);

	fclose (f);

	return 0;
}

unsigned config_read(CONFIG *config, const char *file){
	FILE *f = fopen(file, "r");
	if (!f) {
		perror("open file");
		return 0;
	}

	config_init(config);

	char buf[CONFIG_SIZE];
	int line_number = 0;

	while (fgets(buf, sizeof buf, f)) {
		++line_number;
		int err = config_parse(buf, config);
		if (err) {
			char *errdesc;
			switch (err) {
				case -1: errdesc="Syntax Error"; break;
				case CLIENT_ID_SET: errdesc="Client id already set"; break; 
				case CLIENT_SECRET_SET: errdesc="Client secret already set"; break; 
				case DEVICE_ID_SET: errdesc="Device id already set"; break; 
				case DEVICE_NAME_SET: errdesc="Device name already set"; break; 
				case TOKEN_SET: errdesc="Token already set"; break; 
			}
			fprintf(stderr, "ERROR READ CONFIG LINE %d: %d, %s\n", line_number, err, errdesc);
		} 
	}	
	return config->set;
} 
