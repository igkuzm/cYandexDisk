#config to create cYandexDisk lib with automake
lib_LTLIBRARIES  = libcYandexDisk.la
libcYandexDisk_la_SOURCES = \
		cYandexDisk.c\
		cYandexOAuth.c \
	  	cJSON.c\
	  	uuid4.c

if WINNT
WINDIR = winnt
WINNTFLAG = -DTARGETWINNT 
else
WINDIR = win32
endif

if WIN32
CFLAGS_WIN32 = -I$(WINDIR)/curl/include -I$(WINDIR)/pthreads-win32/include $(WINNTFLAG)
LINKS_WIN32 = -lcurl -L$(WINDIR)/curl/bin -lpthreadGC1 -L$(WINDIR)/pthreads-win32/lib
endif

if WIN64
CFLAGS_WIN64 = -Iwin64/curl/include -Iwin64/pthreads-win32/include
LINKS_WIN64 = -lcurl -Lwin64/curl/bin -lpthreadGC1 -Lwin64/pthreads-win32/lib
endif

if WINDOWS
libcYandexDisk_la_LDFLAGS = -Wl,-no-undefined
else
CURL_LINK = -lcurl
endif

libcYandexDisk_la_CFLAGS = -fPIC $(CFLAGS_WIN32) $(CFLAGS_WIN64)
libcYandexDisk_la_LIBADD = $(CURL_LINK) $(LINKS_WIN32) $(LINKS_WIN64)
