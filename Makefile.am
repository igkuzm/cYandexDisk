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
	-I$(srcdir)/curl/windows/win32/include -I$(srcdir)/pthreads-win32/include
LINKS_WIN32 = -lcurl -L$(srcdir)/curl/windows/win32/bin/libcurl.dll \
	      -lpthreadGC1 -L$(srcdir)/pthreads-win32/lib
endif

if WIN64
libcYandexDisk_la_CFLAGS += \
	-I$(srcdir)/curl/windows/win64/include -I$(srcdir)/pthreads-win32/include
LINKS_WIN64 = -lcurl -L$(srcdir)/curl/windows/win64/bin/libcurl.dll \
              -lpthreadGC1 -L$(srcdir)/pthreads-win32/lib
endif

if WINDOWS
libcYandexDisk_la_LDFLAGS = -Wl,-no-undefined
else
CURL_LINK = -lcurl
endif

libcYandexDisk_la_LIBADD = $(CURL_LINK) $(LINKS_WIN32) $(LINKS_WIN64)
