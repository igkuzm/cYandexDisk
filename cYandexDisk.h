/**
 * File              : cYandexDisk.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 03.05.2022
 * Last Modified Date: 14.07.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
/*
 * C API for Yandex Disk
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

typedef struct c_yd_file_t {
	char   name[256];			//name of resource
	char   type[8];				//type of resource (file, directory)
	char   path[BUFSIZ];		//path of resource in disk
	char   mime_type[64];		
	size_t size;
	time_t created;
	time_t modified;
	char   preview[BUFSIZ];		//url of preview
	char   public_key[BUFSIZ];
	char   public_url[BUFSIZ];
} c_yd_file_t;


//get URL with authoization code request
char *c_yandex_disk_url_to_ask_for_authorization_code(
		const char *client_id,    //id of application in Yandex
		char **error			  //error
);

//get authorization token
void c_yandex_disk_get_token(
		const char *authorization_code, 
		const char *client_id,    //id of application in Yandex
		const char *client_secret,//secret of application in Yandex
		const char *device_name,  //device name
		void * user_data,
		int (*callback)(
			void * user_data,
			char * access_token,
			time_t expires_in,
			char * refresh_token,
			char * error
			)
);

//get info of file/directory
c_yd_file_t *
c_yandex_disk_file_info(
		const char * access_token, 
		const char * path,
		c_yd_file_t *file,
		char **_error
		);


//upload file to Yandex Disk
int c_yandex_disk_upload_file(
		const char * access_token, //authorization token
		const char * filename,     //filename to upload
		const char * path,         //path in yandex disk to save file - start with app:/
		bool overwrite,			   //overwrite distination 
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

//upload data to Yandex Disk
int c_yandex_disk_upload_data(
		const char * access_token, //authorization token
		void * data,			   //data to upload
		size_t size,			   //data size
		const char * path,         //path in yandex disk to save file - start with app:/
		bool overwrite,			   //overwrite distination 
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
		const char * access_token, //authorization token
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

//Download data from Yandex Disk - return data size
int c_yandex_disk_download_data(             
		const char * access_token, //authorization token
		const char * path,         //path in yandex disk of file to download - start with app:/
		void *user_data,           //pointer of data to transfer throw callback
		int (*callback)(		   //callback function when upload finished 
			size_t size,           //size of downloaded data
			void *data,			   //pointer of downloaded data
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
		const char * access_token, //authorization token
		const char * path,		   //path in yandex disk (file or directory)
		void * user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			c_yd_file_t *file,	   //information of resource 
			void * user_data,	   //pointer of data return from callback 
			char * error		   //error
		)
);

//list of shared resources
int c_yandex_disk_ls_public(
		const char * access_token, //authorization token
		void * user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			c_yd_file_t *file,     //information of resource 
			void * user_data,	   //pointer of data return from callback 
			char * error		   //error
		)
);

//get url of file
char *c_yandex_disk_file_url(const char * access_token, const char * path, char **error);

//create directory
int c_yandex_disk_mkdir(const char * access_token, const char * path, char **error);

//remove file/directory
int c_yandex_disk_rm(const char * access_token, const char * path, char **error);

//update resource data
int c_yandex_disk_patch(const char * access_token, const char * path, const char *json_data, char **error);

//copy file from to
int c_yandex_disk_cp(
		const char * access_token, //authorization token
		const char * from,		   //from path in Yandex Disk 
		const char * to,	       //to path in Yandex Disk 
		bool overwrite,			   //overwrite distination 
		void *user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			void *user_data,       //pointer of data return from callback 
			char *error			   //error
		)
);

//move file from to
int c_yandex_disk_mv(
		const char * access_token, //authorization token
		const char * from,		   //from path in Yandex Disk 
		const char * to,	       //to path in Yandex Disk 
		bool overwrite,			   //overwrite distination 
		void *user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			void *user_data,       //pointer of data return from callback 
			char *error			   //error
		)
);

//publish file
int c_yandex_disk_publish(const char * access_token, const char * path, char **error);

//unpublish file
int c_yandex_disk_unpublish(const char * access_token, const char * path, char **error);

//get information of public resource
int c_yandex_disk_public_ls(
		const char * access_token, //authorization token
		const char * public_key,   //key or url of public resource 
		void * user_data,          //pointer of data to transfer throw callback 
		int(*callback)(			   //callback function
			c_yd_file_t *file,     //information of resource
			void * user_data,	   //pointer of data return from callback 
			char * error		   //error
		)
);	

//Download public resources
int c_yandex_disk_download_public_resource(             
		const char * access_token, //authorization token
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
int c_yandex_disk_public_cp(
		const char * access_token, //authorization token
		const char * public_key,   //from path in public resource of Yandex Disk 
		const char * to,	       //to path in Yandex Disk 
		void *user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			void *user_data,       //pointer of data return from callback 
			char *error			   //error
		)
);


#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif
