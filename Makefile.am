#config to create cYandexDisk lib with automake

lib_LTLIBRARIES  = libcYandexDisk.la
libcYandexDisk_la_SOURCES = \
		cYandexDisk.c\
		cYandexOAuth.c \
	  	cJSON.c\
	  	uuid4.c

if WIN32
CFLAGS_WIN32 = \
	-Icurl/windows/win32/include -Ipthreads-win32/include
LINKS_WIN32 = -lcurl -Lcurl/windows/win32/bin \
	      -lpthreadGC1 -Lpthreads-win32/lib
endif

#if WIN64
#libcYandexDisk_la_CFLAGS += \
#	-Icurl/windows/win64/include -Ipthreads-win32/include
#LINKS_WIN64 = -lcurl -Lcurl/windows/win64/bin \
#              -lpthreadGC1 -Lpthreads-win32/lib
#endif

if WINDOWS
libcYandexDisk_la_LDFLAGS = -Wl,-no-undefined
else
CURL_LINK = -lcurl
endif

libcYandexDisk_la_CFLAGS = -fPIC $(CFLAGS_WIN32)
libcYandexDisk_la_LIBADD = $(CURL_LINK) $(LINKS_WIN32) $(LINKS_WIN64)
