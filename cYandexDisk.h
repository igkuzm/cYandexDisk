/**
 * File              : cYandexDisk.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 03.05.2022
 * Last Modified Date: 21.09.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
/*
 * C API for Yandex Disk
 */
#ifndef C_YANDEX_DISK
#define C_YANDEX_DISK

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifndef bool
#define bool char
#define true 1
#define false 0
#endif
#else
#include <stdbool.h>
#endif

#include <stdio.h>
#include <time.h>

// allocate and return URL with verification code request
char __declspec(dllexport) * c_yandex_disk_url_to_ask_for_verification_code(
		const char *client_id,    //id of application in Yandex
		char **error			  //error
);

// parse html, allocate and return verification code from Yandex
char __declspec(dllexport) * c_yandex_disk_verification_code_from_html(
		const char *html,         //html to search verification code
		char **error		      //error
);


// get authorization token
void __declspec(dllexport) c_yandex_disk_get_token(
		const char *verification_code, 
		const char *client_id,    //id of application in Yandex
		const char *client_secret,//secret of application in Yandex
		const char *device_name,  //device name
		void * user_data,
		int (*callback)(
			void * user_data,
			const char * access_token,
			time_t expires_in,
			const char * refresh_token,
			const char * error
			)
);

/* To get yandex disk token - you may ask client to enter code in
 * https://oauth.yandex.ru/device */
void __declspec(dllexport) c_yandex_disk_url_to_ask_for_verification_code_for_user(
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
);

// get authorization token from clients device
void __declspec(dllexport) c_yandex_disk_get_token_from_user(
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
);


/* yandex disk file structure */
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


// get info of file/directory
int __declspec(dllexport) c_yandex_disk_file_info(
		const char * access_token, 
		const char * path,
		c_yd_file_t *file,
		char **_error
		);


//upload file to Yandex Disk
int __declspec(dllexport) c_yandex_disk_upload_file(
		const char * access_token, //authorization token
		FILE *fp,                  //pointer to file read stream
		const char * path,         //path in yandex disk to save file - start with app:/
		bool overwrite,			   //overwrite distination 
		bool wait_finish,
		void *user_data,           //pointer of data to transfer throw callback
		void (*callback)(		   //callback function when upload finished 
			FILE *fp,            
			size_t size,           //size of uploaded file
			void *user_data,       //pointer of data return from callback
			const char *error	   //error
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
int __declspec(dllexport) c_yandex_disk_upload_data(
		const char * access_token, //authorization token
		void * data,			   //data to upload
		size_t size,			   //data size
		const char * path,         //path in yandex disk to save file - start with app:/
		bool overwrite,			   //overwrite distination 
		bool wait_finish,
		void *user_data,           //pointer of data to transfer throw callback
		void (*callback)(		   //callback function when upload finished 
			void *data,
			size_t size,           //size of uploaded file
			void *user_data,       //pointer of data return from callback
			const char *error	   //error
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
int __declspec(dllexport) c_yandex_disk_download_file(             
		const char * access_token, //authorization token
		FILE *fp,                  //pointer to file write stream
		const char * path,         //path in yandex disk of file to download - start with app:/
		bool wait_finish,
		void *user_data,           //pointer of data to transfer throw callback
		void (*callback)(		   //callback function when upload finished 
			FILE *fp,
			size_t size,           //size of downloaded file
			void *user_data,       //pointer of data return from callback
			const char *error	   //error
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
int __declspec(dllexport) c_yandex_disk_download_data(             
		const char * access_token, //authorization token
		const char * path,         //path in yandex disk of file to download - start with app:/
		bool wait_finish,
		void *user_data,           //pointer of data to transfer throw callback
		void (*callback)(		   //callback function when upload finished 
			void *data,			   //pointer of downloaded data
			size_t size,           //size of downloaded data
			void *user_data,       //pointer of data return from callback
			const char *error	   //error
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
int __declspec(dllexport) c_yandex_disk_ls(			   
		const char * access_token, //authorization token
		const char * path,		   //path in yandex disk (file or directory)
		void * user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			const c_yd_file_t *file,	   //information of resource 
			void * user_data,	   //pointer of data return from callback 
			const char * error	   //error
		)
);

//list of shared resources
int __declspec(dllexport) c_yandex_disk_ls_public(
		const char * access_token, //authorization token
		void * user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			const c_yd_file_t *file,     //information of resource 
			void * user_data,	   //pointer of data return from callback 
			const char * error	   //error
		)
);

//allocate and return url of file
char __declspec(dllexport) * c_yandex_disk_file_url(const char * access_token, const char * path, char **error);

//create directory
int __declspec(dllexport) c_yandex_disk_mkdir(const char * access_token, const char * path, char **error);

//remove file/directory
int __declspec(dllexport) c_yandex_disk_rm(const char * access_token, const char * path, char **error);

//update resource data
int __declspec(dllexport) c_yandex_disk_patch(const char * access_token, const char * path, const char *json_data, char **error);

//copy file from to
int __declspec(dllexport) c_yandex_disk_cp(
		const char * access_token, //authorization token
		const char * from,		   //from path in Yandex Disk 
		const char * to,	       //to path in Yandex Disk 
		bool overwrite,			   //overwrite distination 
		void *user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			void *user_data,       //pointer of data return from callback 
			const char *error	   //error
		)
);

//move file from to
int __declspec(dllexport) c_yandex_disk_mv(
		const char * access_token, //authorization token
		const char * from,		   //from path in Yandex Disk 
		const char * to,	       //to path in Yandex Disk 
		bool overwrite,			   //overwrite distination 
		void *user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			void *user_data,       //pointer of data return from callback 
			const char *error	   //error
		)
);

//publish file
int __declspec(dllexport) c_yandex_disk_publish(const char * access_token, const char * path, char **error);

//unpublish file
int __declspec(dllexport) c_yandex_disk_unpublish(const char * access_token, const char * path, char **error);

//get information of public resource
int __declspec(dllexport) c_yandex_disk_public_ls(
		const char * access_token, //authorization token
		const char * public_key,   //key or url of public resource 
		void * user_data,          //pointer of data to transfer throw callback 
		int(*callback)(			   //callback function
			const c_yd_file_t *file,     //information of resource
			void * user_data,	   //pointer of data return from callback 
			const char * error	   //error
		)
);	

//Download public resources
int __declspec(dllexport) c_yandex_disk_download_public_resource(             
		const char * access_token, //authorization token
		FILE *fp,                  //pointer to file write stream
		const char * public_key,   //key or url of public resource 
		bool wait_finish,
		void *user_data,           //pointer of data to transfer throw callback
		void (*callback)(		   //callback function when upload finished 
			FILE *fp,
			size_t size,           //size of downloaded file
			void *user_data,       //pointer of data return from callback
			const char *error      //error
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

int __declspec(dllexport) c_yandex_disk_download_public_resource_data(             
		const char * access_token, //authorization token
		const char * public_key,   //key or url of public resource 
		bool wait_finish,
		void *user_data,           //pointer of data to transfer throw callback
		void (*callback)(		   //callback function when upload finished 
			void *data,
			size_t size,           //size of downloaded file
			void *user_data,       //pointer of data return from callback
			const char *error      //error
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
int __declspec(dllexport) c_yandex_disk_public_cp(
		const char * access_token, //authorization token
		const char * public_key,   //from path in public resource of Yandex Disk 
		const char * to,	       //to path in Yandex Disk 
		void *user_data,		   //pointer of data return from callback 
		int(*callback)(			   //callback function
			void *user_data,       //pointer of data return from callback 
			const char *error	   //error
		)
);

//get information of trash resource
int __declspec(dllexport) c_yandex_disk_trash_ls(
		const char * access_token, //authorization token
		void * user_data,          //pointer of data to transfer throw callback 
		int(*callback)(			       //callback function
			const c_yd_file_t *file, //information of resource
			void * user_data,	       //pointer of data return from callback 
			const char * error	     //error
		)
);	

//restore removed resource
int __declspec(dllexport) c_yandex_disk_trash_restore(const char * access_token, const char * path, char **error);

//clear trash - remove all files
int __declspec(dllexport) c_yandex_disk_trash_empty(const char * access_token, char **error);

// curl functions
int __declspec(dllexport) curl_download_file(FILE *fp, const char * url, void * user_data, void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)); 

size_t __declspec(dllexport) curl_download_data(const char * url, void * user_data, void (*callback)(void *data, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)); 

int __declspec(dllexport) curl_upload_file(FILE *fp, const char * url, void *user_data, void (*callback)(FILE *fp, size_t size, void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow));

int __declspec(dllexport) curl_upload_data(void * data, size_t size, const char * url, void *user_data, void (*callback)(void *data, size_t size,void *user_data, const char *error), void *clientp, int (*progress_callback)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow));

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif

#endif /* ifndef C_YANDEX_DISK */

