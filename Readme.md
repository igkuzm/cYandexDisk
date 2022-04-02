# API for Yandex Disk in C

```
int callback(size_t size, void * user_data, char * error)
{
	if (error)
		fprintf(stderr, "ERROR: %s\n", error);
	if (size > 0)
		fprintf(stderr, "Download complete!\n");
	
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

	c_yandex_disk_download_file("AUTH TOKEN FROM YANDEX", "~/Desktop/test.jpeg", "app:/test.jpeg", NULL, callback, NULL, progress_callback);

	return 0;
}

```
