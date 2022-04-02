/**
 * File              : cYandexDisk.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.03.2022
 * Last Modified Date: 02.04.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "cYandexDisk.h"
#include <curl/curl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "uuid4/uuid4.h"
#include "config.h"
#include <sys/_types/_va_list.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define API_URL "https://cloud-api.yandex.net"
#define VERIFY_SSL 0

// memory allocation helpers
#define MALLOC(size)	\
({	\
	void* const ___p = malloc(size); \
	if(!___p) {perror("Malloc"); exit(EXIT_FAILURE);} \
	___p;\
})

#define REALLOC(ptr, size)	\
({	\
	void* const ___s = ptr; \
	void* const ___p = realloc(___s, size);	\
	if(!___p) { perror("Realloc"); exit(EXIT_FAILURE); }	\
	___p;\
})

#define FREE(ptr) \
({\
	if (ptr)\
		free(ptr);\
	ptr = NULL;\
})

#define NEW(T) ((T*)MALLOC(sizeof(T)))

//error and string helpers
#define ERROR(ptr, ...) ({if(ptr) {*ptr = MALLOC(BUFSIZ); sprintf(*ptr, __VA_ARGS__);};})
#define STR(...) ({char ___str[BUFSIZ]; sprintf(___str, __VA_ARGS__); ___str;})
#define STRCOPY(str0, str1) ({size_t ___size = sizeof(str0); strncpy(str0, str1, ___size - 1); str0[___size - 1] = '\0';})


static char config_path[BUFSIZ] = "cYandexDisk.cfg";
static CONFIG * config;

void c_yandex_disk_set_config_path(const char *_config_path){
	strncpy(config_path, _config_path, BUFSIZ-1);
	config_path[BUFSIZ-1] = '\0';
}

void c_yandex_disk_config_init(const char * config_file_path)
{
	if (config_file_path != NULL) c_yandex_disk_set_config_path(config_file_path);
		
	config = NEW(CONFIG);
	config_init(config);
	config_read(config, config_path);
}

void c_yandex_disk_set_client_id(const char *client_id){
	if (config) {
		config_set(config, client_id, client_id);
		config->set |= CLIENT_ID_SET;
		config_write(config, config_path);
	}
}

void c_yandex_disk_set_client_secret(const char *client_secret){
	if (config) {
		config_set(config, client_secret, client_secret);
		config->set |= CLIENT_SECRET_SET;
		config_write(config, config_path);
	}
}

void c_yandex_disk_set_device_name(const char *device_name){
	if (config) {
		config_set(config, device_name, device_name);
		config->set |= DEVICE_NAME_SET;
		config_write(config, config_path);
	}
}

int c_yandex_disk_set_device_id(){
	if (config) {
		UUID4_STATE_T state;
		UUID4_T uuid;

		uuid4_seed(&state);
		uuid4_gen(&state, &uuid);

		if (!uuid4_to_s(uuid, config->device_id, 37)){
			perror("Can't genarate UUID");
			return -1;
		}

		config->set |= DEVICE_ID_SET;
		config_write(config, config_path);

		return 0;
	}
	
	return -1;
}

void c_yandex_disk_set_token(const char *token){
	if (config) {
		config_set(config, token, token);
		config->set |= TOKEN_SET;
		config_write(config, config_path);
	}
}


int check_no_config(char **error){
	if (!config) {
		ERROR(error, "Error. c_yandex_disk is not initilized, you should run c_yandex_disk_init()\n");
		return -1;
	}	
	return 0;
}

int check_no_client_id(char **error){
	if (check_no_config(error))	return -1;
	if (!(config->set & CLIENT_ID_SET)) {
		ERROR(error, "Error. Client id is not set. You should run c_yandex_disk_set_client_id(client_id)\n");	
		return -1;
	}
	return 0;
}

int check_no_client_secret(char **error){
	if (check_no_config(error))	return -1;
	if (!(config->set & CLIENT_SECRET_SET)) {
		ERROR(error, "Error. Client secret is not set. You should run c_yandex_disk_set_client_secret(client_secret)\n");	
		return -1;
	}
	return 0;
}

int check_no_client_device_name(char **error){
	if (check_no_config(error))	return -1;
	if (!(config->set & DEVICE_NAME_SET)) {
		ERROR(error, "Error. Device name is not set. You should run c_yandex_disk_set_device_name(device_name)\n");	
		return -1;
	}
	return 0;
}

int check_no_client_device_id(char **error){
	if (config) {
		if (!(config->set & DEVICE_ID_SET)) {
			if (c_yandex_disk_set_device_id()){
				ERROR(error, "Error. Device id is not set. Can't set UUID\n");	
				return -1;
			}
		}
	}
	return 0;
}

int check_no_token(char **error){
	if (check_no_config(error))	return -1;
	if (!(config->set & TOKEN_SET)) {
		ERROR(error, "Error. Token is not set. You should get token from Yandex. Get authorization code first: open URL c_yandex_disk_ask_for_authorization_code(&error) to enable access of application and get code. Then get token with c_yandex_disk_get_token(authorization_code, &error)\n");	
		return -1;
	}
	return 0;
}

struct string {
	char *ptr;
	size_t len;
};

void init_string(struct string *s) {
	s->len = 0;
	s->ptr = MALLOC(s->len+1);
	s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = REALLOC(s->ptr, new_len+1);
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

char *c_yandex_disk_url_to_ask_for_authorization_code(const char *client_id,  char **error) {
	
	if (client_id == NULL) {
		if (check_no_client_id(error))	return NULL;
		client_id = config->client_id; 
	}

	char *requestString = MALLOC(BUFSIZ);
	
	sprintf(requestString, "\"https://oauth.yandex.ru/authorize?response_type=code");	
	sprintf(requestString, "%s&client_id=%s\"", requestString, client_id);
	
	return requestString;
}

char *c_yandex_disk_get_token(const char *authorization_code, const char *client_id, const char *client_secret, const char *device_name, char **error){
	if (authorization_code != NULL) {
		ERROR(error, "No authorization_code.");
		return NULL;
	}

	if (client_id == NULL) {
		if (check_no_client_id(error))			return NULL;
		client_id = config->client_id; 
	}
	if (client_secret == NULL) {
		if (check_no_client_secret(error))		return NULL;
		client_secret = config->client_secret;
	}
	if (device_name == NULL) {	
		if (check_no_client_device_name(error))	return NULL;
		device_name = config->device_name;
	}
	char *device_id; 
	char device_uuid[37];
	UUID4_STATE_T state; UUID4_T uuid;
	uuid4_seed(&state);
	uuid4_gen(&state, &uuid);
	if (!uuid4_to_s(uuid, device_uuid, 37)){
		ERROR(error, "Can't genarate UUID");
		return NULL;
	}
	device_id = device_uuid;
	
	if (device_name == NULL) {	
		if (check_no_client_device_id(error))	    return NULL;
		device_id = config->device_id;
	}

	CURL *curl = curl_easy_init();
		
	struct string s;
	init_string(&s);

	if(curl) {
		char requestString[] = "https://oauth.yandex.ru/token";	
		
		curl_easy_setopt(curl, CURLOPT_URL, requestString);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");		
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);

		struct curl_slist *header = NULL;
	    header = curl_slist_append(header, "Connection: close");		
	    header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded");		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		
		char post[BUFSIZ];
		sprintf(post, "grant_type=authorization_code");		
		sprintf(post, "%s&code=%s",				post, authorization_code);
		sprintf(post, "%s&client_id=%s",		post, client_id);
		sprintf(post, "%s&client_secret=%s",	post, client_secret);
		sprintf(post, "%s&device_id=%s",		post, device_id);
		sprintf(post, "%s&device_name=%s",		post, device_name);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post));
	    
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_slist_free_all(header);
		if (res) { //handle erros
			ERROR(error, "CURL ERROR: %d", res);
			free(s.ptr);
            return NULL;			
		}		
		//parse JSON answer
		cJSON *json = cJSON_Parse(s.ptr);
		free(s.ptr);
		if (cJSON_IsObject(json)) {
			cJSON *access_token = cJSON_GetObjectItem(json, "access_token");			
			if (!access_token) { //handle errors
				cJSON *error_description = cJSON_GetObjectItem(json, "error_description");
				if (!error_description) {
					ERROR(error, "UNKNOWN ERROR!"); //no error code in JSON answer
					cJSON_free(json);
					return NULL;
				}
				ERROR(error, "CONNECTION ERROR: %s", error_description->valuestring);
				cJSON_free(json);
				return NULL;
			}
			//OK - we have a token
			char * token = MALLOC(BUFSIZ);
			strncpy(token, access_token->valuestring, BUFSIZ - 1);	
			token[BUFSIZ - 1] = '\0';
			if (config)
				c_yandex_disk_set_token(access_token->valuestring);
			cJSON_free(json);
			return token;
		}	
	}
	return NULL;
}

int curl_download_file(const char * filename, const char * url, void * user_data, int (*callback)(size_t size, void *user_data, char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)) 
{
    FILE *fd;
	
	fd = fopen(filename, "wb"); /* open file to download */
	if(!fd){
		callback(0,user_data, STR("Error download file. Can't open destination file\n"));
		return 1; /* cannot continue */
	}
    
	CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
		
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);		
		/* enable verbose for easier tracing */
		/*curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);*/
		/* example.com is redirected, so we tell libcurl to follow redirection */
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);		
		if (progress_callback) {
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, clientp);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		}
			
        res = curl_easy_perform(curl);
        fclose(fd);

		if(res != CURLE_OK) {
			callback(0,user_data, STR("curl_easy_perform() failed: %s\n", curl_easy_strerror(res)));
			curl_easy_cleanup(curl);
			return -1;
		} else {
			/* now extract transfer info */
			curl_off_t size;
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
			callback(size, user_data, NULL);
		}	
        /* always cleanup */
		curl_easy_cleanup(curl);
    }
    return 0;
}

int curl_upload_file(const char * filename, const char * url, void *user_data, int (*callback)(size_t size, void *user_data, char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	CURL *curl;
	CURLcode res;
	struct stat file_info;
	FILE *fd;

	fd = fopen(filename, "rb"); /* open file to upload */
	if(!fd){
		callback(0,user_data, "Error upload file. Can't open file\n");
		return 1; /* cannot continue */
	}

	/* to get the file size */
	if(fstat(fileno(fd), &file_info) != 0){
		callback(0,user_data, "Error upload file. File has zero size\n");
		return 1; /* cannot continue */
	}

	curl = curl_easy_init();
	if(curl) {
		/* upload to this place */
		curl_easy_setopt(curl, CURLOPT_URL, url);

		/* tell it to "upload" to the URL */
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* set where to read from (on Windows you need to use READFUNCTION too) */
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);

		/* and give the size of the upload (optional) */
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

		/* enable verbose for easier tracing */
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		if (progress_callback) {
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, clientp);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		}		

		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK) {
			callback(0,user_data, STR("curl_easy_perform() failed: %s\n", curl_easy_strerror(res)));
			curl_easy_cleanup(curl);
			return -1;
		}
		else {
			/* now extract transfer info */
			curl_off_t size;
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD_T, &size);
			callback(size, user_data, NULL);
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	fclose(fd);
	return 0;
}

cJSON *c_yandex_disk_api(const char * http_method, const char *api_suffix, const char * token, char **error, ...)
{
	if (token == NULL) {
		token=config->token;
		if (check_no_token(error)) return NULL;
	}
	char authorization[BUFSIZ];
	sprintf(authorization, "Authorization: OAuth %s", token);

	CURL *curl = curl_easy_init();
		
	struct string s;
	init_string(&s);
	
	if(curl) {
		char requestString[BUFSIZ];	
		sprintf(requestString, "%s/%s", API_URL, api_suffix);
		va_list argv;
		va_start(argv, error);
		char *arg = va_arg(argv, char*);
		if (arg) {
			sprintf(requestString, "%s?%s", requestString, arg);
			arg = va_arg(argv, char*);	
		}
		while (arg) {
			sprintf(requestString, "%s&%s", requestString, arg);
			arg = va_arg(argv, char*);	
		}
		va_end(argv);
		
		curl_easy_setopt(curl, CURLOPT_URL, requestString);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, http_method);		
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);

		/* enable verbose for easier tracing */
		/*curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);		*/

		struct curl_slist *header = NULL;
	    header = curl_slist_append(header, "Connection: close");		
	    header = curl_slist_append(header, "Content-Type: application/json");
	    header = curl_slist_append(header, "Accept: application/json");
	    header = curl_slist_append(header, authorization);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_slist_free_all(header);
		if (res) { //handle erros
			ERROR(error, "CURL ERROR: %d", res);
			free(s.ptr);
            return NULL;			
		}		
		//parse JSON answer
		cJSON *json = cJSON_Parse(s.ptr);
		free(s.ptr);		

		return json;
	}

	free(s.ptr);
	return NULL;
}

typedef enum {
	FILE_DOWNLOAD,
	FILE_UPLOAD
} FILE_TRANSFER;

struct curl_transfer_file_in_thread_params {
	FILE_TRANSFER file_transfer;
	const char *filename;
	char url[BUFSIZ];
	void *user_data;
	int (*callback)(size_t size, void *user_data, char *error);
	void *clientp;
	int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
};

void *curl_transfer_file_in_thread(void *_params)
{
	struct curl_transfer_file_in_thread_params *params = _params;

	switch (params->file_transfer) {
		case FILE_UPLOAD :
			curl_upload_file(params->filename, params->url, params->user_data, params->callback, params->clientp, params->progress_callback);
			break;
		case FILE_DOWNLOAD :
			curl_download_file(params->filename, params->url, params->user_data, params->callback, params->clientp, params->progress_callback);
			break;			
	}

	free(params);
	pthread_exit(0);
}

int _c_yandex_disk_transfer_file_parser(cJSON *json, FILE_TRANSFER file_transfer, const char *filename, char *error, void *user_data, int (*callback)(size_t size, void *user_data, char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	if (!json) {
		callback(0,user_data,STR("CONNECTION ERROR: %s", error));
		return -1;
	}
	cJSON *url = cJSON_GetObjectItem(json, "href");			
	if (!url) {
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		callback(0,user_data,STR("CONNECTION ERROR: %s", message->valuestring));
		cJSON_free(json);
		return  -1;
	}

	//upload file in new thread
	pthread_t tid; //идентификатор потока
	pthread_attr_t attr; //атрибуты потока

	//получаем дефолтные значения атрибутов
	int err = pthread_attr_init(&attr);
	if (err) {
		perror("THREAD attributes");
		return err;
	}	

	//set params
	struct curl_transfer_file_in_thread_params *params = NEW(struct curl_transfer_file_in_thread_params);
	params->filename = filename;
	strcpy(params->url, url->valuestring);
	params->user_data = user_data;
	params->callback = callback;
	params->file_transfer = file_transfer;
	params->clientp = clientp;
	params->progress_callback = progress_callback;
	//создаем новый поток
	err = pthread_create(&tid,&attr, curl_transfer_file_in_thread, params);
	if (err) {
		perror("create THREAD");
		return err;
	}

	return 0;
}

int c_yandex_disk_upload_file(const char * token, const char * filename, const char * path, void *user_data, int (*callback)(size_t size, void *user_data, char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);

	char *error;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/upload", token, &error, path_arg, NULL);

	return _c_yandex_disk_transfer_file_parser(json, FILE_UPLOAD, filename, error, user_data, callback, clientp, progress_callback);
}

int c_yandex_disk_download_file(const char * token, const char * filename, const char * path, void *user_data, int (*callback)(size_t size, void *user_data, char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/download", token, &error, path_arg, NULL);
	return _c_yandex_disk_transfer_file_parser(json, FILE_DOWNLOAD, filename, error, user_data, callback, clientp, progress_callback);
}

int c_yandex_disk_download_public_resource(const char * token, const char * filename, const char * public_key, void *user_data, int (*callback)(size_t size, void *user_data, char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char public_key_arg[BUFSIZ];
	sprintf(public_key_arg, "public_key=%s", public_key);	

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/public/resources/download", token, &error, public_key_arg, NULL);
	return _c_yandex_disk_transfer_file_parser(json, FILE_DOWNLOAD, filename, error, user_data, callback, clientp, progress_callback);
}

int c_json_to_c_yd_file_t(cJSON *json, c_yd_file_t *file)
{
	cJSON *name = cJSON_GetObjectItem(json, "name");	
	if (name) STRCOPY(file->name, name->valuestring);

	cJSON *type = cJSON_GetObjectItem(json, "type");	
	if (type) STRCOPY(file->type, type->valuestring);	

	cJSON *path = cJSON_GetObjectItem(json, "path");	
	if (path) STRCOPY(file->path, path->valuestring);	

	cJSON *mime_type = cJSON_GetObjectItem(json, "mime_type");	
	if (mime_type) STRCOPY(file->mime_type, mime_type->valuestring);	

	cJSON *size = cJSON_GetObjectItem(json, "size");	
	if (size) file->size = size->valueint;	

	cJSON *preview = cJSON_GetObjectItem(json, "preview");	
	if (preview) STRCOPY(file->preview, preview->valuestring);	

	cJSON *public_key = cJSON_GetObjectItem(json, "public_key");	
	if (public_key) STRCOPY(file->public_key, public_key->valuestring);	
	
	cJSON *public_url = cJSON_GetObjectItem(json, "public_url");	
	if (public_url) STRCOPY(file->public_url, public_url->valuestring);	
	
	return 0;
}

int _c_yandex_disk_ls_parser(cJSON *json, char *error, void * user_data, int(*callback)(c_yd_file_t *file, void * user_data, char * error))
{
	if (!json) { //no json returned
		callback(NULL,user_data,STR("CONNECTION ERROR: %s", error));
		return -1;
	}
	if (!cJSON_GetObjectItem(json, "path")){ //error to get info of file/directory
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		callback(NULL,user_data,STR("CONNECTION ERROR: %s", message->valuestring));
		cJSON_free(json);
		return  -1;
	}	
	cJSON *items = cJSON_GetObjectItem(json, "items");
	if (items) { //we have items in directory
		int count = cJSON_GetArraySize(items);
		for (int i = 0; i < count; ++i) {
			cJSON *item = cJSON_GetArrayItem(items, i);
			c_yd_file_t file;
			c_json_to_c_yd_file_t(item, &file);
			callback(&file, user_data, NULL);				
		}
		
	} else { //no items - return file info
		c_yd_file_t file;
		c_json_to_c_yd_file_t(json, &file);
		callback(&file, user_data, NULL);
	}	

	return 0;
}	

int c_yandex_disk_ls(const char * token, const char * path, void * user_data, int(*callback)(c_yd_file_t *file, void * user_data, char * error))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources", token, &error, path_arg, NULL);
	
	return _c_yandex_disk_ls_parser(json, error, user_data, callback);
}

int c_yandex_disk_ls_public(const char * token, void * user_data, int(*callback)(c_yd_file_t *file, void * user_data, char * error))
{
	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/public", token, &error, NULL);
	
	return _c_yandex_disk_ls_parser(json, error, user_data, callback);
}

int _c_yandex_disk_standart_parser(cJSON *json, char **error){
	if (!json) //no json returned
		return -1;

	if (!cJSON_GetObjectItem(json, "href")){ //error to get info
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		ERROR(error, "CONNECTION ERROR: %s", message->valuestring);
		cJSON_free(json);
		return  -1;		
	}	
	cJSON_free(json);
	return 0;	
}


int _c_yandex_disk_async_operation(const char * token, const char *operation_id, void *user_data, int(*callback)(void *user_data, char *error))
{
	char url_suffix[BUFSIZ];
	sprintf(url_suffix, "v1/disk/operations/%s", operation_id);

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", url_suffix, token, &error, NULL);	

	if (!json) { //no json returned
		callback(user_data,STR("CONNECTION ERROR: %s", error));
		return -1;
	}
	if (!cJSON_GetObjectItem(json, "href")){ //error to get info
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		callback(user_data,STR("CONNECTION ERROR: %s", message->valuestring));
		cJSON_free(json);
		return  -1;
	}	

	return 0;
}

struct _c_yandex_disk_async_parser_params {
	char operation_id[BUFSIZ];
	const char *token;
	void *user_data;
	int(*callback)(void *user_data, char *error);
};

void * _c_yandex_disk_async_operation_in_thead(void *_params)
{
	struct _c_yandex_disk_async_parser_params *params = _params;
	_c_yandex_disk_async_operation(params->token, params->operation_id, params->user_data, params->callback);
	free(params);
	pthread_exit(0);	
}

int _c_yandex_disk_async_parser(cJSON *json, const char * token, void *user_data, int(*callback)(void *user_data, char *error)){
	if (!json) //no json returned
		return -1;

	cJSON *operation_id = cJSON_GetObjectItem(json, "href"); 
	if (!operation_id){ //error to get info
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		callback(user_data, STR("CONNECTION ERROR: %s", message->valuestring));
		cJSON_free(json);
		return  -1;		
	}	

	pthread_t tid; //идентификатор потока
	pthread_attr_t attr; //атрибуты потока

	//получаем дефолтные значения атрибутов
	int err = pthread_attr_init(&attr);
	if (err) {
		perror("THREAD attributes");
		cJSON_free(json);
		return err;
	}	

	//set params
	struct _c_yandex_disk_async_parser_params *params = NEW(struct _c_yandex_disk_async_parser_params);
	strcpy(params->operation_id, operation_id->valuestring);
	params->user_data = user_data;
	params->callback = callback;
	params->token = token;

	cJSON_free(json);
	
	//создаем новый поток
	err = pthread_create(&tid,&attr, _c_yandex_disk_async_operation_in_thead, params);
	if (err) {
		perror("create THREAD");
		return err;
	}	

	return 0;	
}

int c_yandex_disk_mkdir(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	cJSON *json = c_yandex_disk_api("PUT", "v1/disk/resources", token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_cp(const char * token, const char * from, const char * to, bool overwrite, void *user_data, int(*callback)(void *user_data, char *error))
{
	char from_arg[BUFSIZ];
	sprintf(from_arg, "from=%s", from);	
	
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", to);	

	char overwrite_arg[32];
	sprintf(overwrite_arg, "overwrite=%s", overwrite ? "true" : "false");		

	char async_arg[] = "force_async=true";

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("POST", "v1/disk/resources/copy", token, &error, from_arg, path_arg, overwrite_arg, async_arg, NULL);
	if (error) callback(user_data, error);
	return _c_yandex_disk_async_parser(json, token, user_data, callback);
}

int c_yandex_disk_mv(const char * token, const char * from, const char * to, bool overwrite, void *user_data, int(*callback)(void *user_data, char *error))
{
	char from_arg[BUFSIZ];
	sprintf(from_arg, "from=%s", from);	
	
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", to);	

	char overwrite_arg[32];
	sprintf(overwrite_arg, "overwrite=%s", overwrite ? "true" : "false");		

	char async_arg[] = "force_async=true";

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("POST", "v1/disk/resources/move", token, &error, from_arg, path_arg, overwrite_arg, async_arg, NULL);
	if (error) callback(user_data, error);
	return _c_yandex_disk_async_parser(json, token, user_data, callback);
}

int c_yandex_disk_publish(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	cJSON *json = c_yandex_disk_api("PUT", "v1/disk/resources/publish", token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_unpublish(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	cJSON *json = c_yandex_disk_api("PUT", "v1/disk/resources/unpublish", token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_public_ls(const char * token, const char * public_key, void * user_data, int(*callback)(c_yd_file_t *file, void * user_data, char * error))
{
	char public_key_arg[BUFSIZ];
	sprintf(public_key_arg, "public_key=%s", public_key);	
	
	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/public/resources", token, &error, public_key_arg, NULL);
	return _c_yandex_disk_ls_parser(json, error, user_data, callback);
}

int c_yandex_disk_public_cp(const char * token, const char * public_key, const char * to, void *user_data, int(*callback)(void *user_data, char *error))
{
	char public_key_arg[BUFSIZ];
	sprintf(public_key_arg, "public_key=%s", public_key);	
	
	char save_path_arg[BUFSIZ];
	sprintf(save_path_arg, "save_path=%s", to);	

	char async_arg[] = "force_async=true";

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("POST", "v1/disk/resources/copy", token, &error, public_key_arg, save_path_arg, async_arg, NULL);
	if (error) callback(user_data, error);
	return _c_yandex_disk_async_parser(json, token, user_data, callback);
}