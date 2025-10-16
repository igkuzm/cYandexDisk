#include "cYandexDisk.h"
#include <pthread.h>

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

int main(int argc, char *argv[])
{
  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&tid,&attr,potok,argv[1]);

  c_yandex_disk_url_to_ask_for_verification_code_for_user(
														  "", "test", NULL, callback);
  return 0;
}
