#config to create cYandexDisk lib with automake

lib_LTLIBRARIES  = libcYandexDisk.la
libcYandexDisk_la_SOURCES = \
		cYandexDisk.c\
		cYandexOAuth.c \
	  	cJSON.c\
	  	uuid4.c

libcYandexDisk_la_CFLAGS = -fPIC

if WIN32
libcYandexDisk_la_CFLAGS += \
	-Icurl/windows/win32/include -Ipthreads-win32/include
LINKS_WIN32 = -lcurl -Lcurl/windows/win32/bin/libcurl.dll \
	      -lpthreadGC1 -Lpthreads-win32/lib
endif

if WIN64
libcYandexDisk_la_CFLAGS += \
	-Icurl/windows/win64/include -Ipthreads-win32/include
LINKS_WIN64 = -lcurl -Lcurl/windows/win64/bin/libcurl.dll \
              -lpthreadGC1 -Lpthreads-win32/lib
endif

if WINDOWS
libcYandexDisk_la_LDFLAGS = -Wl,-no-undefined
else
CURL_LINK = -lcurl
endif

libcYandexDisk_la_LIBADD = $(CURL_LINK) $(LINKS_WIN32) $(LINKS_WIN64)
