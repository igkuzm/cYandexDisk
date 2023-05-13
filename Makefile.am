#config to create cYandexDisk lib with automake

lib_LTLIBRARIES  = libcYandexDisk.la
libcYandexDisk_la_SOURCES = \
		cYandexDisk.c\
	  cJSON.c\
	  uuid4.c

CURL_LINK =

if WINDOWS
if X86_64
		CURL_LINKS += curl/windows/win64/lib/libcurl.dll.a
		INCLUDES += curl/windows/win64/include
endif
if I386
		CURL_LINKS += curl/windows/win32/lib/libcurl.dll.a
		INCLUDES += curl/windows/win32/include
endif
else
	CURL_LINK += -lcurl
endif

libcYandexDisk_la_LIBADD = $(CURL_LINK)
