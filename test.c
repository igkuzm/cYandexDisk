/**
 * File              : test.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.03.2022
 * Last Modified Date: 02.04.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "cYandexDisk.h"
#include <stdio.h>
#include <stdlib.h>

char *readString(){
	int i = 0;
	char *a = (char *) calloc(BUFSIZ, sizeof(char));
	if (!a) {
		fprintf(stderr, "ERROR. Cannot allocate memory\n");		
		return NULL;
	}										
	
	while (1) {
		scanf("%c", &a[i]);
		if (a[i] == '\n') {
			break;
		}
		else {
			i++;
		}
	}
	a[i] = '\0';
	return a;
}

int callback(size_t size, void * user_data, char * error)
{
	fprintf(stderr, "CALLBACK FROM DOWNLOAD\n");
	if (error) {
		fprintf(stderr, "ERROR: %s\n", error);
	}

	printf("SIZE: %ld\n", size);

	if (size > 0) {
		fprintf(stderr, "Download complete!\n");
	}
	return 0;
}

int progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	fprintf(stderr, "DOWNLOADED: %f%%\n", dlnow/dltotal*100);

	return 0;
}

int main(int argc, char *argv[])
{
	printf("cYandexDisk Test\n");

	c_yandex_disk_download_file("AQAAAABVlmyCAAfIlGdowLE_UEJ3s5VLRpHRwPo", "/Users/kuzmich/Desktop/test.jpeg", "app:/test.jpeg", NULL, callback, NULL, progress_callback);

	//char *error;
	//char *token = c_yandex_disk_get_token(authorization_code, &error);
	//if (token) {
		//printf("TOKEN: %s\n", token);
	//} else {
		//printf("ERROR: %s\n", error);
	//}
	//char *error = NULL;
	//curl_upload_file("/Users/kuzmich/Desktop/test.jpeg", "https://uploader31j.disk.yandex.net:443/upload-target/20220330T142559.513.utd.devgm90fmdovbwhum7t82tngy-k31j.15225401", &error);

	//if (error)
		//printf("ERROR: %s\n", error);
	
	getchar();
	
	return 0;
}