#config to create cYandexDisk lib with automake

lib_LTLIBRARIES  = libcYandexDisk.la
libcYandexDisk_la_SOURCES = \
		cYandexDisk.c\
		cYandexOAuth.c \
	  	cJSON.c\
	  	uuid4.c

libcYandexDisk_la_CFLAGS = -fPIC

if WIN32
AM_CFLAGS = \
	-Icurl/windows/win32/include
CURL_LINK = -lcurl -L`pwd`/curl/windows/win32/bin/libcurl.dll
endif

if WIN64
AM_CFLAGS = \
	-Icurl/windows/win64/include
CURL_LINK = -lcurl -L`pwd`/curl/windows/win64/bin/libcurl.dll
endif

if WINDOWS
libcYandexDisk_la_SOURCES += strptime.cc
libcYandexDisk_la_LDFLAGS = -Wl,-no-undefined
else
CURL_LINK = -lcurl
endif

libcYandexDisk_la_LIBADD = $(CURL_LINK)
