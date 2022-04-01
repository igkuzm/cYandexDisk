/**
 * File              : cYandexDisk.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.03.2022
 * Last Modified Date: 01.04.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stdio.h>
#include <stdbool.h>

typedef struct c_yd_file_t {
	char   name[256];			//name of resource
	char   type[8];				//type of resource (file, directory)
	char   path[BUFSIZ];		//path of resource in disk
	char   mime_type[64];		
	size_t size;
	char   preview[BUFSIZ];		//url of preview
	char   public_key[BUFSIZ];
	char   public_url[BUFSIZ];
} c_yd_file_t;

//init API. To use default config file (pwd/cYandexDisk.cfg) pass NULL
void c_yandex_disk_init(const char * config_file_path);

//set config client_id (id of applocation in Yandex - to )
void c_yandex_disk_set_client_id(const char *_client_id);
//set config client_secret (secret of application in Yandex)
void c_yandex_disk_set_client_secret(const char *_client_secret);
//device name
void c_yandex_disk_set_device_name(const char *_device_name);
//device id - generated uuid string
int  c_yandex_disk_set_device_id();

//get URL with authoization code request
char *c_yandex_disk_url_to_ask_for_authorization_code(char **error);

//get authorization token
char *c_yandex_disk_get_token(const char *authorization_code, char **error);

//set token in config
void c_yandex_disk_set_token(const char *token);

//upload file to Yandex Disk
int c_yandex_disk_upload_file(
		const char * filename,     //filename to upload
		const char * path,         //path in yandex disk to save file - start with app:/
		void *user_data,           //pointer of data to transfer throw callback
		int (*callback)(		   //callback function when upload finished 
			size_t size,           //size of uploaded file
			void *user_data,       //pointer of data return from callback
			char *error			   //error
		), 
		void *clientp,			   //data pointer to transfer trow progress callback
		int (*progress_callback)(  //progress callback function
			void *clientp,		   //data pointer return from progress function
			double dltotal,        //downloaded total size
			double dlnow,		   //downloaded size
			double ultotal,        //uploaded total size
			double ulnow           //uploaded size
		)
);

//Download file from Yandex Disk
int c_yandex_disk_download_file(             
		const char * filename,     //filename to save downloaded file
		const char * path,         //path in yandex disk of file to download - start with app:/
		void *user_data,           //pointer of data to transfer throw callback
		int (*callback)(		   //callback function when upload finished 
			size_t size,           //size of downloaded file
			void *user_data,       //pointer of data return from callback
			char *error			   //error
		), 
		void *clientp,			   //data pointer to transfer trow progress callback
		int (*progress_callback)(  //progress callback function
			void *clientp,		   //data pointer return from progress function
			double dltotal,        //downloaded total size
			double dlnow,		   //downloaded size
			double ultotal,        //uploaded total size
			double ulnow           //uploaded size
		)
);

//list directory or get info of file
int c_yandex_disk_ls(
		const char * path, 
		void * user_data, 
		int(*callback)(
			c_yd_file_t *file, 
			void * user_data, 
			char * error
		)
);

//list of shared resources
int c_yandex_disk_ls_public(
		void * user_data, 
		int(*callback)(
			c_yd_file_t *file, 
			void * user_data, 
			char * error
		)
);

//create directory
int c_yandex_disk_mkdir(const char * path, char **error);

//copy file from to
int c_yandex_disk_cp(const char * from, const char * to, bool overwrite, void *user_data, int(*callback)(void *user_data, char *error));

//move file from to
int c_yandex_disk_mv(const char * from, const char * to, bool overwrite, void *user_data, int(*callback)(void *user_data, char *error));

//publish file
int c_yandex_disk_publish(const char * path, char **error);

//unpublish file
int c_yandex_disk_unpublish(const char * path, char **error);

//get information of public resource
int c_yandex_disk_public_ls(
		const char * public_key,   //key or url of public resource 
		void * user_data,          //pointer of data to transfer throw callback 
		int(*callback)(
			c_yd_file_t *file, 
			void * user_data, 
			char * error
		)
);	

//Download public resources
int c_yandex_disk_download_public_resource(             
		const char * filename,     //filename to save downloaded file
		const char * public_key,   //key or url of public resource 
		void *user_data,           //pointer of data to transfer throw callback
		int (*callback)(		   //callback function when upload finished 
			size_t size,           //size of downloaded file
			void *user_data,       //pointer of data return from callback
			char *error			   //error
		), 
		void *clientp,			   //data pointer to transfer trow progress callback
		int (*progress_callback)(  //progress callback function
			void *clientp,		   //data pointer return from progress function
			double dltotal,        //downloaded total size
			double dlnow,		   //downloaded size
			double ultotal,        //uploaded total size
			double ulnow           //uploaded size
		)
);

//copy public resource to Yandex Disk
int c_yandex_disk_public_cp(const char * public_key, const char * to, void *user_data, int(*callback)(void *user_data, char *error));


