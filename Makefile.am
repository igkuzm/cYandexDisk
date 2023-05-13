#config to create static cYandexDisk lib with automake

noinst_LIBRARIES = libcYandexDisk.a

libkdata2_a_SOURCES = \
		cYandexDisk.c\
	  cJSON.c\
	  uuid4.c

CURL_LINK =

if WINDOWS
if I386
		CURL_LINKS += curl/windows/win64/lib/libcurl.dll.a
		INCLUDE += curl/windows/win64/include
endif
if I386
		CURL_LINKS += curl/windows/win32/lib/libcurl.dll.a
		INCLUDE += curl/windows/win32/include
endif
else
	CURL_LINK += -lcurl
endif

libkdata2_a_LIBADD = $(CURL_LINK)
