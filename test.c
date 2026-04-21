#include "cYandexDisk.h"
#include "cYandexOAuth.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>


#define CLIENTID "3e35f6472d6843f2bab4438d74678184"
#define CLIENTSECRET "8814ec4a327d46d0a814a869898c1bfc"

static int callback(
			void * user_data,
			const char * device_code,
			const char * user_code,
			const char * verification_url,
			int interval,
			int expires_in,
			const char * error)
{
  if (error)
	printf("%s\n" ,error);
}

void *potok(void *param){
  printf("statrt in thread\n");
}

static int token_callback(
			void * user_data,
			const char * access_token,
			int expires_in,
			const char * refresh_token,
			const char * error
			)
{
	if (error){
		printf("ERROR: %s\n", error);
		return 0;
	}
	
	printf("OK! Got token: %s", access_token);

	FILE *fp = fopen("token", "w");
	if (fp){
		fwrite(access_token, strlen(access_token), 1, fp);
		fclose(fp);
	}
	
	return 0;
}

static int code_callback(
			void * user_data,
			const char * device_code,
			const char * user_code,
			const char * verification_url,
			int interval,
			int expires_in,
			const char * error
			)
{
	if (error){
		printf("ERROR: %s\n", error);
		return 0;
	}

	printf("open %s \nand enter code: %s\n", 
			verification_url, user_code);

	c_yandex_oauth_get_token_from_user(
			device_code, 
			CLIENTID, 
			CLIENTSECRET, 
			interval, 
			expires_in, 
			NULL, 
			token_callback);

	return 0;
}

int main(int argc, char *argv[])
{
  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&tid,&attr,potok,argv[1]);

	c_yandex_oauth_code_from_user(
				CLIENTID, 
			  "cYandexDisk", 
				NULL, 
				code_callback);

  return 0;
}
