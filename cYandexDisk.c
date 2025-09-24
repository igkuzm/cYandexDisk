/**
 * File              : cYandexDisk.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 03.05.2022
 * Last Modified Date: 07.10.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "cYandexDisk.h"
#include <curl/curl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "uuid4.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include "alloc.h"
#include "log.h"
#include "str.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define STRCOPY(dst, src)\
	({strncpy(dst, src, sizeof(dst)-1); dst[sizeof(dst)-1]=0;})

//add strptime
/*char * strptime(const char* s, const char* f, struct tm* tm);*/

#define API_URL "https://cloud-api.yandex.net"
#define VERIFY_SSL 0

#define YD_ANSWER_LIMIT 20

char * 
c_yandex_disk_url_to_ask_for_verification_code(
		const char *client_id,  char **err)
{
	char *requestString = MALLOC(BUFSIZ, 
		 if (err)
			*err = strdup("malloc");
			return NULL);
	
	sprintf(requestString, 
			"https://oauth.yandex.ru/authorize?response_type=code");	
	sprintf(requestString, 
			"%s&client_id=%s", requestString, client_id);
	
	return requestString;
}

char *
c_yandex_disk_verification_code_from_html(
		const char *html,         //html to search verification code
		char **err		            //error
		)
{
	const char * patterns[] = {
		"verification_code%3Fcode%3D",
		"class=\"verification-code-code\">"
	};
	const char *pattern_ends[] = {
		"&",
		"<"
	};	

	int i;
	for (int i = 0; i < 2; i++) {
		const char * s = patterns[i]; 
		int len = strlen(s);

		//find start of verification code class structure in html
		char *start = strstr(html, s); 
		if (!start){
			if (err)
				*err = strdup("HTML has no verification code class");
		} else {
			//find end of code
			char *end = 
				strstr(start, pattern_ends[i]);
			if (!end)
				*err = strdup(STR("no pattern ends: %s", pattern_ends[i]));
			else {
				//find length of verification code
				long clen = end - start;

				//allocate code and copy
				char * code = MALLOC(clen + 1, 
						if(err)
							*err = strdup("malloc");
						return NULL);
				strncpy(code, start, clen);
				code[clen] = 0;

				return code;
			}
		}
	}
	return NULL;
}

size_t writefunc(
		void *ptr, size_t size, size_t nmemb, struct str *s)
{
	str_append(s, ptr, size*nmemb);
	return size*nmemb;
}

void c_yandex_disk_url_to_ask_for_verification_code_for_user(
		const char *client_id, //id of application in Yandex
		const char *device_name,  //device name
		void * user_data,
		int (*callback)(
			void * user_data,
			const char * device_code,
			const char * user_code,
			const char * verification_url,
			int interval,
			int expires_in,
			const char * error
			)
		)
{
	if (client_id == NULL) {
		callback(user_data, NULL, NULL, NULL, 0, 0, "cYandexDisk: No client_id");
		return;
	}
	
	if (device_name == NULL) {
		callback(user_data, NULL, NULL, NULL, 0, 0, "cYandexDisk: No deivce_name");
		return;
	}

	char device_id[37];
	uuid4_init();
	uuid4_generate(device_id);
	
	CURL *curl = curl_easy_init();
		
	struct str s;
	str_init(&s);

	if(curl) {
		char requestString[] = "https://oauth.yandex.ru/device/code";	
		
		curl_easy_setopt(curl, CURLOPT_URL, requestString);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");		
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);

		struct curl_slist *header = NULL;
	    header = curl_slist_append(header, "Connection: close");		
	    header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded");		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		
		char post[BUFSIZ];
		sprintf(post, "%s&client_id=%s",		post, client_id);
		sprintf(post, "%s&device_id=%s",		post, device_id);
		sprintf(post, "%s&device_name=%s",	post, device_name);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post));
	    
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_slist_free_all(header);
		if (res) { //handle erros
			callback(user_data, NULL, NULL, NULL, 0, 0, STR("cYandexDisk: curl returned error: %d", res));
			free(s.str);
            return;			
		}		
		//parse JSON answer
		cJSON *json = 
			cJSON_ParseWithLength(s.str, s.len);
		free(s.str);
		if (cJSON_IsObject(json)) {
			cJSON *device_code = cJSON_GetObjectItem(json, "device_code");			
			if (!device_code) { //handle errors
				cJSON *error_description = cJSON_GetObjectItem(json, "error_description");
				if (!error_description) {
					callback(user_data, NULL, NULL, NULL, 0, 0, STR("cYandexDisk: unknown error!")); //no error code in JSON answer
					cJSON_free(json);
					return;
				}
				callback(user_data, NULL, NULL, NULL, 0, 0, STR("cYandexDisk: %s", error_description->valuestring)); //no error code in JSON answer
				cJSON_free(json);
				return;
			}
			//OK - we have a code
			callback(user_data, 
					device_code->valuestring, 
					cJSON_GetObjectItem(json, "user_code")->valuestring, 
					cJSON_GetObjectItem(json, "verification_url")->valuestring, 
					cJSON_GetObjectItem(json, "interval")->valueint, 
					cJSON_GetObjectItem(json, "expires_in")->valueint, 
					NULL
					);
			cJSON_free(json);
		}	
	}

}

void c_yandex_disk_get_token_from_user(
		const char *device_code, 
		const char *client_id,    //id of application in Yandex
		const char *client_secret,//secret of application in Yandex
		int interval,
		int expires_in,
		void * user_data,
		int (*callback)(
			void * user_data,
			const char * access_token,
			time_t expires_in,
			const char * refresh_token,
			const char * error
			)
	)
{	
	if (device_code == NULL) {
		callback(user_data, NULL, 0, NULL, "cYandexDisk: No device_code");
		return;
	}
	if (client_id == NULL) {
		callback(user_data, NULL, 0, NULL, "cYandexDisk: No client_id");
		return;
	}
	if (client_secret == NULL) {
		callback(user_data, NULL, 0, NULL, "cYandexDisk: No client_secret");
		return;
	}

	char device_id[37];
	uuid4_init();
	uuid4_generate(device_id);
	
	CURL *curl = curl_easy_init();
		
	struct str s;
	str_init(&s);

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
		sprintf(post, "grant_type=device_code");		
		sprintf(post, "%s&code=%s",				  post, device_code);
		sprintf(post, "%s&client_id=%s",		post, client_id);
		sprintf(post, "%s&client_secret=%s",post, client_secret);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post));
	    
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		

		int i;
		for (i=0; i < expires_in; i += interval){
			
			CURLcode res = curl_easy_perform(curl);

			curl_easy_cleanup(curl);
			curl_slist_free_all(header);
			if (res) { //handle erros
				callback(user_data, NULL, 0, NULL, STR("cYandexDisk: curl returned error: %d", res));
				free(s.str);
							continue;			
			}		
			//parse JSON answer
			cJSON *json = 
				cJSON_ParseWithLength(s.str, s.len);
			free(s.str);
			if (cJSON_IsObject(json)) {
				cJSON *access_token = cJSON_GetObjectItem(json, "access_token");			
				if (!access_token) { //handle errors
					cJSON *error_description = cJSON_GetObjectItem(json, "error_description");
					if (!error_description) {
						callback(user_data, NULL, 0, NULL, STR("cYandexDisk: unknown error!")); //no error code in JSON answer
						cJSON_free(json);
						continue;
					}
					callback(user_data, NULL, 0, NULL, STR("cYandexDisk: %s", error_description->valuestring)); //no error code in JSON answer
					cJSON_free(json);
					continue;
				}
				//OK - we have a token
				callback(user_data, access_token->valuestring, cJSON_GetObjectItem(json, "expires_in")->valueint, cJSON_GetObjectItem(json, "refresh_token")->valuestring, NULL);
				cJSON_free(json);
				break;
			}	
#ifdef _WIN32
			Sleep(interval*1000);
#else
			sleep(interval);
#endif
		}
	}
}
void 
c_yandex_disk_get_token(
		const char *verification_code, 
		const char *client_id, 
		const char *client_secret, 
		const char *device_name, 
		void * user_data,
		int (*callback)(
			void * user_data,
			const char * access_token,
			time_t expires_in,
			const char * refresh_token,
			const char * error
			)
		)
{
	if (verification_code == NULL) {
		callback(user_data, NULL, 0, NULL, "cYandexDisk: No verification_code");
		return;
	}

	char device_id[37];
	uuid4_init();
	uuid4_generate(device_id);
	
	CURL *curl = curl_easy_init();
		
	struct str s;
	str_init(&s);

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
		sprintf(post, "%s&code=%s",				post, verification_code);
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
			callback(user_data, NULL, 0, NULL, STR("cYandexDisk: curl returned error: %d", res));
			free(s.str);
            return;			
		}		
		//parse JSON answer
		cJSON *json = 
			cJSON_ParseWithLength(s.str, s.len);
		free(s.str);
		if (cJSON_IsObject(json)) {
			cJSON *access_token = cJSON_GetObjectItem(json, "access_token");			
			if (!access_token) { //handle errors
				cJSON *error_description = cJSON_GetObjectItem(json, "error_description");
				if (!error_description) {
					callback(user_data, NULL, 0, NULL, STR("cYandexDisk: unknown error!")); //no error code in JSON answer
					cJSON_free(json);
					return;
				}
				callback(user_data, NULL, 0, NULL, STR("cYandexDisk: %s", error_description->valuestring)); //no error code in JSON answer
				cJSON_free(json);
				return;
			}
			//OK - we have a token
			callback(user_data, access_token->valuestring, cJSON_GetObjectItem(json, "expires_in")->valueint, cJSON_GetObjectItem(json, "refresh_token")->valuestring, NULL);
			cJSON_free(json);
		}	
	}
}

int curl_download_file(FILE *fp, const char * url, void * user_data, void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)) 
{
	
	CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
		
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);		
		/* enable verbose for easier tracing */
		/*curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);*/
		/* example.com is redirected, so we tell libcurl to follow redirection */
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);		
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		
		if (progress_callback) {
#if LIBCURL_VERSION_NUM < 0x073200
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, clientp);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#else
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, clientp);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#endif			
		}
			
        res = curl_easy_perform(curl);
        fclose(fp);

		if(res != CURLE_OK) {
			if (callback)
				callback(fp, 0,user_data, STR("cYandexDisk: curl_easy_perform() failed: %d(%s)\n",res, curl_easy_strerror(res)));
			curl_easy_cleanup(curl);
			return -1;
		} else {
			/* now extract transfer info */
			curl_off_t size;
#if LIBCURL_VERSION_NUM >= 0x071904
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
#else
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
#endif

			if (callback)
				callback(fp, size, user_data, NULL);
		}	
        /* always cleanup */
		curl_easy_cleanup(curl);
    }
    return 0;
}

size_t curl_download_data_writefunc(
		void *data, size_t size, size_t nmemb, struct str *s)
{
	str_append(s, data, size * nmemb);
	return size*nmemb;
}

size_t curl_download_data(const char * url, void * user_data, void (*callback)(void *data, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)) 
{
	CURL *curl;
    CURLcode res;

	struct str s;
	str_init(&s);

    curl = curl_easy_init();
    if (curl) {
		
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_download_data_writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		/* enable verbose for easier tracing */
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		/* example.com is redirected, so we tell libcurl to follow redirection */
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);		
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		
		if (progress_callback) {
#if LIBCURL_VERSION_NUM < 0x073200
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, clientp);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#else
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, clientp);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#endif			
		}
			
        res = curl_easy_perform(curl);

		if(res != CURLE_OK) {
			if (callback)
				callback(NULL, 0, user_data, STR("cYandexDisk: curl_easy_perform() failed: %d(%s)\n", res, curl_easy_strerror(res)));
			curl_easy_cleanup(curl);
			////return -1;
		} else {
			/* now extract transfer info */
			curl_off_t size;
#if LIBCURL_VERSION_NUM >= 0x071904
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
#else
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
#endif
			if (callback)
				callback(s.str, size, user_data, NULL);
		}	
        /* always cleanup */
		curl_easy_cleanup(curl);
    }
	free(s.str);
    return s.len;
}

size_t curl_upload_file_readfunc(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	FILE *readhere = (FILE *)userdata;
	curl_off_t nread;

	/* copy as much data as possible into the 'ptr' buffer, but no more than
	 'size' * 'nmemb' bytes! */
	size_t retcode = fread(ptr, size, nmemb, readhere);

	nread = (curl_off_t)retcode;

	//fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);
	return retcode;
}

int curl_upload_file(FILE *fp, const char * url, void *user_data, void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	CURL *curl;
	CURLcode res;
	struct stat file_info;

	/* to get the file size */
	if(fstat(fileno(fp), &file_info) != 0){
		if (callback)
			callback(fp, 0,user_data, "Error upload file. File has zero size\n");
		return 1; /* cannot continue */
	}

	curl = curl_easy_init();
	if(curl) {
		/* upload to this place */
		curl_easy_setopt(curl, CURLOPT_URL, url);

		/* tell it to "upload" to the URL */
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* set where to read from (on Windows you need to use READFUNCTION too) */
		curl_easy_setopt(curl, CURLOPT_READDATA, fp);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, curl_upload_file_readfunc);

		/* and give the size of the upload (optional) */
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

		/* enable verbose for easier tracing */
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		
		
		if (progress_callback) {
#if LIBCURL_VERSION_NUM < 0x073200
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, clientp);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#else
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, clientp);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#endif			
		}		

		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK) {
			if (callback)
				callback(fp, 0,user_data, STR("cYandexDisk: curl_easy_perform() failed: %d(%s)\n", res, curl_easy_strerror(res)));
			curl_easy_cleanup(curl);
			return -1;
		}
		else {
			/* now extract transfer info */
			curl_off_t size;
#if LIBCURL_VERSION_NUM >= 0x071904
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD_T, &size);
#else
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD, &size);
#endif
			if (callback)
				callback(fp, size, user_data, NULL);
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return 0;
}

struct memory {
	void *data;
	int size;
};

size_t curl_upload_data_readfunc(
		char *ptr, size_t size, size_t nmemb, struct memory *t)
{
	size_t s = size * nmemb;
	
	if (s > t->size)
		s = t->size;

	memcpy(ptr, t->data, s);
	
	t->data += s;
	t->size -= s;

	return s;
}

int curl_upload_data(void * data, size_t size, const char * url, void *user_data, void (*callback)(void *data, size_t size,void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	CURL *curl;
	CURLcode res;

	struct memory t;	
	t.data = data;
	t.size = size;

	curl = curl_easy_init();
	if(curl) {
		/* upload to this place */
		curl_easy_setopt(curl, CURLOPT_URL, url);

		/* tell it to "upload" to the URL */
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* set where to read from (on Windows you need to use READFUNCTION too) */
		curl_easy_setopt(curl, CURLOPT_READDATA, &t);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, curl_upload_data_readfunc);

		/* and give the size of the upload (optional) */
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, size);

		/* enable verbose for easier tracing */
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		

		if (progress_callback) {
#if LIBCURL_VERSION_NUM < 0x073200
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, clientp);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#else
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, clientp);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
#endif			
		}		

		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK) {
			if (callback)
				callback(data, 0, user_data, STR("cYandexDisk: curl_easy_perform() failed: %d(%s)\n", res, curl_easy_strerror(res)));
			curl_easy_cleanup(curl);
			return -1;
		}
		else {
			/* now extract transfer info */
			curl_off_t size;
#if LIBCURL_VERSION_NUM >= 0x071904
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD_T, &size);
#else
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD, &size);
#endif
			if (callback)
				callback(data, size, user_data, NULL);
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return 0;
}

cJSON *c_yandex_disk_api(const char * http_method, const char *api_suffix, const char *body, const char * token, char **error, ...)
{
	char authorization[BUFSIZ];
	sprintf(authorization, "Authorization: OAuth %s", token);

	CURL *curl = curl_easy_init();
		
	struct str s;
	str_init(&s);
	
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
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);		

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

		if (body) {
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(body));
		}

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, VERIFY_SSL);		

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_slist_free_all(header);
		if (res) { //handle erros
			if (error)
				*error = strdup(STR("cYandexDisk: curl returned error: %d", res));
			free(s.str);
        return NULL;			
		}		
		//parse JSON answer
		cJSON *json = cJSON_ParseWithLength(s.str, s.len);
		free(s.str);		

		return json;
	}

	free(s.str);
	return NULL;
}

typedef enum {
	FILE_DOWNLOAD,
	FILE_UPLOAD,
	DATA_UPLOAD,
	DATA_DOWNLOAD
} FILE_TRANSFER;

struct curl_transfer_file_in_thread_params {
	FILE_TRANSFER file_transfer;
	FILE *fp;
	char url[BUFSIZ];
	void *user_data;
	void (*callback)(FILE *fp, size_t size, void *user_data, const char *error);
	void (*callback_data)(void *data, size_t size, void *user_data, const char *error);
	void *clientp;
	int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	void *data;
	size_t size;
};

void *curl_transfer_file_in_thread(void *_params)
{
	struct curl_transfer_file_in_thread_params *params = _params;

	switch (params->file_transfer) {
		case FILE_UPLOAD :
			curl_upload_file(params->fp, params->url, params->user_data, params->callback, params->clientp, params->progress_callback);
			break;
		case FILE_DOWNLOAD :
			curl_download_file(params->fp, params->url, params->user_data, params->callback, params->clientp, params->progress_callback);
			break;			
		case DATA_UPLOAD :
			curl_upload_data(params->data, params->size, params->url, params->user_data, params->callback_data, params->clientp, params->progress_callback);
			break;
		case DATA_DOWNLOAD :
			curl_download_data(params->url, params->user_data, params->callback_data, params->clientp, params->progress_callback);			
			break;			
	}

	free(params);
	pthread_exit(0);
}

int  _c_yandex_disk_transfer_file_parser(cJSON *json, FILE_TRANSFER file_transfer, bool wait_finish, FILE *fp, void * data, size_t size, char *error, void *user_data, void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), void (*callback_data)(void *data, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	if (!json) {
		if (callback)
			callback(fp, 0,user_data,STR("cYandexDisk: %s", error));
		return -1;
	}
	cJSON *url = cJSON_GetObjectItem(json, "href");			
	if (!url) {
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		if (callback)
			callback(fp, 0,user_data,STR("cYandexDisk: %s", message->valuestring));
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
		if (callback)
			callback(fp, 0,user_data,STR("cYandexDisk: %s", "THREAD attributes"));
		return err;
	}	

	//set params
	struct curl_transfer_file_in_thread_params *params = 
		NEW(struct curl_transfer_file_in_thread_params, return -1);
	params->fp = fp;
	strcpy(params->url, url->valuestring);
	params->user_data = user_data;
	params->callback = callback;
	params->file_transfer = file_transfer;
	params->clientp = clientp;
	params->progress_callback = progress_callback;
	params->data = data;
	params->size = size;
	params->callback_data = callback_data;
	//создаем новый поток
	err = pthread_create(&tid,&attr, curl_transfer_file_in_thread, params);
	if (err) {
		perror("create THREAD");
		if (callback)
			callback(fp, 0,user_data,STR("cYandexDisk: %s", "create THREAD"));
		return err;
	}

	if (wait_finish){
		//connect to thread and wait finish
		err = pthread_join(tid, NULL);
		if (err) {
			if (callback)
				callback(fp, 0,user_data,STR("Error in THREAD: %d\n", err));
		}	
	}

	return 0;
}

char *
c_yandex_disk_file_url(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);

	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/download", NULL, token, error, path_arg, NULL);
	if (!json) //no json returned
		return NULL;

	cJSON *href = cJSON_GetObjectItem(json, "href");
	if (!href){ //error to get info
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		if (error)
			*error = strdup(STR("cYandexDisk: %s", message->valuestring));
		cJSON_free(json);
		return  NULL;		
	}	
	size_t size = strlen(href->valuestring);
	char *url = MALLOC(BUFSIZ, return NULL);
	strncpy(url, href->valuestring, size);
	url[size] = '\0';
	cJSON_free(json);
	return url;
}

int c_yandex_disk_upload_file(const char * token, FILE *fp, const char * path, bool overwrite, bool wait_finish, void *user_data, void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);

	char overwrite_arg[32];
	sprintf(overwrite_arg, "overwrite=%s", overwrite ? "true" : "false");		

	char *error;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/upload", NULL, token, &error, path_arg, overwrite_arg, NULL);

	return _c_yandex_disk_transfer_file_parser(json, FILE_UPLOAD, wait_finish, fp, NULL, 0, error, user_data, callback, NULL, clientp, progress_callback);
}

int c_yandex_disk_upload_data(const char * token, void * data, size_t size, const char * path, bool overwrite, bool wait_finish, void *user_data, void (*callback)(void *data, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);
	
	char overwrite_arg[32];
	sprintf(overwrite_arg, "overwrite=%s", overwrite ? "true" : "false");		

	char *error;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/upload", NULL, token, &error, path_arg, overwrite_arg, NULL);

	return _c_yandex_disk_transfer_file_parser(json, DATA_UPLOAD, wait_finish, NULL, data, size, error, user_data, NULL, callback, clientp, progress_callback);
}

int c_yandex_disk_download_file(const char * token, FILE *fp, const char * path, bool wait_finish, void *user_data, void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/download", NULL, token, &error, path_arg, NULL);
	return _c_yandex_disk_transfer_file_parser(json, FILE_DOWNLOAD, wait_finish, fp, NULL, 0, error, user_data, callback, NULL, clientp, progress_callback);
}

int c_yandex_disk_download_data(const char * token, const char * path, bool wait_finish, void *user_data, void (*callback)(void *data, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/download", NULL, token, &error, path_arg, NULL);
	return _c_yandex_disk_transfer_file_parser(json, DATA_DOWNLOAD, wait_finish, NULL, NULL, 0, error, user_data, NULL, callback, clientp, progress_callback);
}

int c_yandex_disk_download_public_resource(
		const char * token, 
		FILE *fp, 
		const char * public_key, 
		bool wait_finish, 
		void *user_data, 
		void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), 
		void *clientp, 
		int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char public_key_arg[BUFSIZ];
	sprintf(public_key_arg, "public_key=%s", public_key);	

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/public/resources/download", NULL, token, &error, public_key_arg, NULL);
	return _c_yandex_disk_transfer_file_parser(json, FILE_DOWNLOAD, wait_finish, fp, NULL, 0, error, user_data, callback, NULL, clientp, progress_callback);
}

int c_yandex_disk_download_public_resource_data(const char * token, const char * public_key, bool wait_finish, void *user_data, void (*callback)(void *data, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow))
{
	char public_key_arg[BUFSIZ];
	sprintf(public_key_arg, "public_key=%s", public_key);	

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", "v1/disk/public/resources/download", NULL, token, &error, public_key_arg, NULL);
	return _c_yandex_disk_transfer_file_parser(json, DATA_DOWNLOAD, wait_finish, NULL, NULL, 0, error, user_data, NULL, callback, clientp, progress_callback);
}
int c_json_to_c_yd_file_t(cJSON *json, c_yd_file_t *file)
{
	file->name[0] = '\0';
	cJSON *name = cJSON_GetObjectItem(json, "name");	
	if (name) STRCOPY(file->name, name->valuestring);

	file->type[0] = '\0';
	cJSON *type = cJSON_GetObjectItem(json, "type");	
	if (type) STRCOPY(file->type, type->valuestring);	

	file->path[0] = '\0';
	cJSON *path = cJSON_GetObjectItem(json, "path");	
	if (path) STRCOPY(file->path, path->valuestring);	

	file->mime_type[0] = '\0';
	cJSON *mime_type = cJSON_GetObjectItem(json, "mime_type");	
	if (mime_type) STRCOPY(file->mime_type, mime_type->valuestring);	

	file->size = 0;
	cJSON *size = cJSON_GetObjectItem(json, "size");	
	if (size) file->size = size->valueint;	

	file->preview[0] = '\0';
	cJSON *preview = cJSON_GetObjectItem(json, "preview");	
	if (preview) STRCOPY(file->preview, preview->valuestring);	

	file->public_key[0] = '\0';
	cJSON *public_key = cJSON_GetObjectItem(json, "public_key");	
	if (public_key) STRCOPY(file->public_key, public_key->valuestring);	
	
	file->public_url[0] = '\0';
	cJSON *public_url = cJSON_GetObjectItem(json, "public_url");	
	if (public_url) STRCOPY(file->public_url, public_url->valuestring);	

	file->modified = 0;
	cJSON *modified = cJSON_GetObjectItem(json, "modified");	
	if (modified) {
		struct tm tm = {0};
		sscanf(modified->valuestring, "%d-%d-%dT%d:%d:%d+00:00", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
		tm.tm_year -= 1900; //struct tm year starts from 1900
		tm.tm_mon -= 1; //struct tm mount start with 0 for January
		tm.tm_isdst = 0; //should not use summer time flag		
		//strptime(modified->valuestring, "%FT%T%z", &tm);
		file->modified = mktime(&tm);
	}	

	file->created = 0;
	cJSON *created = cJSON_GetObjectItem(json, "created");	
	if (created) {
		struct tm tm = {0};
		sscanf(modified->valuestring, "%d-%d-%dT%d:%d:%d+00:00", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
		tm.tm_year -= 1900; //struct tm year starts from 1900
		tm.tm_mon -= 1; //struct tm mount start with 0 for January
		tm.tm_isdst = 0; //should not use summer time flag
		/*strptime(modified->valuestring, "%FT%T%z", &tm);*/
		file->created = mktime(&tm);
	}		
	
	return 0;
}

int _c_yandex_disk_ls_parser(cJSON *json, char *error, void * user_data, int(*callback)(const c_yd_file_t *file, void * user_data, const char * error))
{
	if (!json) { //no json returned
		if (callback)
			callback(NULL,user_data,STR("cYandexDisk: %s", error));
		return -1;
	}
	if (!cJSON_GetObjectItem(json, "path")){ //error to get info of file/directory
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		if (callback)
			callback(NULL,user_data,STR("cYandexDisk: %s", message->valuestring));
		cJSON_free(json);
		return  -1;
	}	
	cJSON *items = NULL;
	cJSON *_embedded = cJSON_GetObjectItem(json, "_embedded");
	if (_embedded)
		items = cJSON_GetObjectItem(_embedded, "items");
	else 
		items = cJSON_GetObjectItem(json, "items");
	if (items) { //we have items in directory
		int count = cJSON_GetArraySize(items);
		if (!count)
			return 1;
		for (int i = 0; i < count; ++i) {
			cJSON *item = cJSON_GetArrayItem(items, i);
			c_yd_file_t file;
			c_json_to_c_yd_file_t(item, &file);
			if (callback)
				callback(&file, user_data, NULL);				
		}
	} else { //no items - return file info
		c_yd_file_t file;
		c_json_to_c_yd_file_t(json, &file);
		if (callback)
			callback(&file, user_data, NULL);
	}	

	return 0;
}	
int
c_yandex_disk_file_info(
		const char * token, 
		const char * path,
		c_yd_file_t *file,
		char **_error
		)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	char *error = NULL;
	cJSON *json = 
		c_yandex_disk_api("GET", "v1/disk/resources", NULL, token, &error, path_arg, NULL);
	if (error && _error) {
		*_error = error;
	}

	if (!json) { //no json returned
		return -1;
	}
	if (!cJSON_GetObjectItem(json, "path")){ //error to get info of file/directory
		cJSON *message = cJSON_GetObjectItem(json, "error");
		if (_error)
			*_error = strdup(message->valuestring);
		cJSON_free(json);
		return -1;
	}	
	
	if (file)
		c_json_to_c_yd_file_t(json, file);
	return 0;
}

int c_yandex_disk_ls(const char * token, const char * path, void * user_data, int(*callback)(const c_yd_file_t *file, void * user_data, const char * error))
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	int i = 0, r = 0, l = YD_ANSWER_LIMIT;
	char limit[BUFSIZ], offset[BUFSIZ];
	
	sprintf(limit, "limit=%d", l);
	
	while	(r == 0) {
		sprintf(offset, "offset=%d", i++ * l);
		char *error = NULL;
		cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources", NULL, token, &error, path_arg, limit, offset, NULL);
		r = _c_yandex_disk_ls_parser(json, error, user_data, callback);
	}
	return r;
}

int c_yandex_disk_ls_public(const char * token, void * user_data, int(*callback)(const c_yd_file_t *file, void * user_data, const char * error))
{
	
	int i = 0, r = 0, l = YD_ANSWER_LIMIT;
	char limit[BUFSIZ], offset[BUFSIZ];
	
	sprintf(limit, "limit=%d", l);
	
	while	(r == 0) {
		sprintf(offset, "offset=%d", i++ * l);
		char *error = NULL;
		cJSON *json = c_yandex_disk_api("GET", "v1/disk/resources/public", NULL, token, &error, limit, offset, NULL);
		r = _c_yandex_disk_ls_parser(json, error, user_data, callback);
	}
	return r;
}

int _c_yandex_disk_standart_parser(cJSON *json, char **error){
	if (!json) //no json returned
		return -1;

	if (!cJSON_GetObjectItem(json, "href") && !cJSON_GetObjectItem(json, "path")){ //error to get info
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		if (error)
			*error = strdup(STR("cYandexDisk: %s", message->valuestring));
		cJSON_free(json);
		return  -1;		
	}	
	cJSON_free(json);
	return 0;	
}


int _c_yandex_disk_async_operation(const char * token, const char *operation_id, void *user_data, int(*callback)(void *user_data, const char *error))
{
	char url_suffix[BUFSIZ];
	sprintf(url_suffix, "v1/disk/operations/%s", operation_id);

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("GET", url_suffix, NULL, token, &error, NULL);	

	if (!json) { //no json returned
		if (callback)
			callback(user_data,STR("cYandexDisk: %s", error));
		return -1;
	}
	if (!cJSON_GetObjectItem(json, "href")){ //error to get info
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		if (callback)
			callback(user_data,STR("cYandexDisk: %s", message->valuestring));
		cJSON_free(json);
		return  -1;
	}	

	return 0;
}

struct _c_yandex_disk_async_parser_params {
	char operation_id[BUFSIZ];
	const char *token;
	void *user_data;
	int(*callback)(void *user_data, const char *error);
};

void * _c_yandex_disk_async_operation_in_thead(void *_params)
{
	struct _c_yandex_disk_async_parser_params *params = _params;
	_c_yandex_disk_async_operation(params->token, params->operation_id, params->user_data, params->callback);
	free(params);
	pthread_exit(0);	
}

int _c_yandex_disk_async_parser(cJSON *json, const char * token, void *user_data, int(*callback)(void *user_data, const char *error)){
	if (!json) //no json returned
		return -1;

	cJSON *operation_id = cJSON_GetObjectItem(json, "href"); 
	if (!operation_id){ //error to get info
		cJSON *message = cJSON_GetObjectItem(json, "message");			
		if (callback)
			callback(user_data, STR("cYandexDisk: %s", message->valuestring));
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
	struct _c_yandex_disk_async_parser_params *params = 
		NEW(struct _c_yandex_disk_async_parser_params, return -1);
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

	cJSON *json = c_yandex_disk_api("PUT", "v1/disk/resources", NULL, token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_rm(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	cJSON *json = c_yandex_disk_api("DELETE", "v1/disk/resources", NULL, token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_patch(const char * token, const char * path, const char *json_data, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	cJSON *json = c_yandex_disk_api("PATCH", "v1/disk/resources", json_data, token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
	return 0;
}

int c_yandex_disk_cp(const char * token, const char * from, const char * to, bool overwrite, void *user_data, int(*callback)(void *user_data, const char *error))
{
	char from_arg[BUFSIZ];
	sprintf(from_arg, "from=%s", from);	
	
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", to);	

	char overwrite_arg[32];
	sprintf(overwrite_arg, "overwrite=%s", overwrite ? "true" : "false");		

	char async_arg[] = "force_async=true";

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("POST", "v1/disk/resources/copy", NULL, token, &error, from_arg, path_arg, overwrite_arg, async_arg, NULL);
	if (error) callback(user_data, error);
	return _c_yandex_disk_async_parser(json, token, user_data, callback);
}

int c_yandex_disk_mv(const char * token, const char * from, const char * to, bool overwrite, void *user_data, int(*callback)(void *user_data, const char *error))
{
	char from_arg[BUFSIZ];
	sprintf(from_arg, "from=%s", from);	
	
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", to);	

	char overwrite_arg[32];
	sprintf(overwrite_arg, "overwrite=%s", overwrite ? "true" : "false");		

	char async_arg[] = "force_async=true";

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("POST", "v1/disk/resources/move", NULL, token, &error, from_arg, path_arg, overwrite_arg, async_arg, NULL);
	if (error) callback(user_data, error);
	return _c_yandex_disk_async_parser(json, token, user_data, callback);
}

int c_yandex_disk_publish(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	cJSON *json = c_yandex_disk_api("PUT", "v1/disk/resources/publish", NULL, token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_unpublish(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	

	cJSON *json = c_yandex_disk_api("PUT", "v1/disk/resources/unpublish", NULL, token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_public_ls(const char * token, const char * public_key, void * user_data, int(*callback)(const c_yd_file_t *file, void * user_data, const char * error))
{
	char public_key_arg[BUFSIZ];
	sprintf(public_key_arg, "public_key=%s", public_key);	

	int i = 0, r = 0, l = YD_ANSWER_LIMIT;
	char limit[BUFSIZ], offset[BUFSIZ];
	
	sprintf(limit, "limit=%d", l);
	
	while	(r == 0) {
		sprintf(offset, "offset=%d", i++ * l);
		char *error = NULL;
		cJSON *json = c_yandex_disk_api("GET", "v1/disk/public/resources", NULL, token, &error, public_key_arg, limit, offset, NULL);
		r = _c_yandex_disk_ls_parser(json, error, user_data, callback);
	}
	return r;
}

int c_yandex_disk_public_cp(const char * token, const char * public_key, const char * to, void *user_data, int(*callback)(void *user_data, const char *error))
{
	char public_key_arg[BUFSIZ];
	sprintf(public_key_arg, "public_key=%s", public_key);	
	
	char save_path_arg[BUFSIZ];
	sprintf(save_path_arg, "save_path=%s", to);	

	char async_arg[] = "force_async=true";

	char *error = NULL;
	cJSON *json = c_yandex_disk_api("POST", "v1/disk/resources/copy", NULL, token, &error, public_key_arg, save_path_arg, async_arg, NULL);
	if (error) 
		if (callback)
			callback(user_data, error);
	return _c_yandex_disk_async_parser(json, token, user_data, callback);
}

int c_yandex_disk_trash_ls(
		const char * token, 
		void * user_data,          
		int(*callback)(			       
			const c_yd_file_t *file, 
			void * user_data,	       
			const char * error)
		)
{
	int i = 0, r = 0, l = YD_ANSWER_LIMIT;
	char limit[BUFSIZ], offset[BUFSIZ];
	
	sprintf(limit, "limit=%d", l);
	
	while	(r == 0) {
		sprintf(offset, "offset=%d", i++ * l);
		char *error = NULL;
		cJSON *json = c_yandex_disk_api("GET", "v1/disk/trash/resources", NULL, token, &error, limit, offset, NULL);
		r = _c_yandex_disk_ls_parser(json, error, user_data, callback);
	}
	return r;

}	

int c_yandex_disk_trash_restore(const char * token, const char * path, char **error)
{
	char path_arg[BUFSIZ];
	sprintf(path_arg, "path=%s", path);	
	cJSON *json = c_yandex_disk_api("PUT", "v1/disk/trash/resources", NULL, token, error, path_arg, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}

int c_yandex_disk_trash_empty(const char * token, char **error)
{
	cJSON *json = c_yandex_disk_api("DELETE", "v1/disk/trash/resources", NULL, token, error, NULL);
	return _c_yandex_disk_standart_parser(json, error);
}
