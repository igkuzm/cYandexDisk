#config to create cYandexDisk lib with automake

lib_LTLIBRARIES  = libcYandexDisk.la
libcYandexDisk_la_SOURCES = \
		cYandexDisk.c\
	  cJSON.c\
	  uuid4.c

if WIN32
AM_CFLAGS = \
	-Icurl/windows/win32/include
CURL_LINK = curl/windows/win32/lib/libcurl.dll.a
endif

if WIN64
AM_CFLAGS = \
	-Icurl/windows/win64/include
CURL_LINK = curl/windows/win64/lib/libcurl.dll.a
endif

if !WINDOWS
CURL_LINK = -lcurl
endif

libcYandexDisk_la_LIBADD = $(CURL_LINK)
