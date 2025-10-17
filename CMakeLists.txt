cmake_minimum_required(VERSION 3.15)

set(TARGET cYandexDisk)

#To include cYandexDisk to your project use:
#set(CYANDEXDISK_SOURCE_DIR ${CMAKE_SOURCE_DIR}/path/to/cYandexDisk)
#add_subdirectory(${CYANDEXDISK_SOURCE_DIR})

if (NOT DEFINED CYANDEXDISK_SOURCE_DIR)
  set(CYANDEXDISK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif ()

project(
	${TARGET} VERSION 1.1 
	DESCRIPTION "C API for Yandex Disk"
	HOMEPAGE_URL ""
	#LANGUAGES C CXX 
	LANGUAGES C 
)

set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 17)

if(APPLE)
	SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -framework Foundation")
endif()

# Determine the location of the libraries to use based on the platform.
if(ANDROID)
	ADD_LIBRARY(curl STATIC IMPORTED)
	SET_TARGET_PROPERTIES(curl PROPERTIES IMPORTED_LOCATION
		${CYANDEXDISK_SOURCE_DIR}/android/curl/${ANDROID_ABI}/libcurl.a)
	INCLUDE_DIRECTORIES(${CYANDEXDISK_SOURCE_DIR}/android/curl/include)
	set(LIBZ z)
elseif(IOS)
	ADD_LIBRARY(curl STATIC IMPORTED)
	SET_TARGET_PROPERTIES(curl PROPERTIES IMPORTED_LOCATION 
		${CYANDEXDISK_SOURCE_DIR}/iOS/curl/libcurl.a)
	INCLUDE_DIRECTORIES(${CYANDEXDISK_SOURCE_DIR}/iOS/curl/include)
elseif(WIN32)
	if (CMAKE_BUILD_TYPE STREQUAL "WINNT")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTARGETWINNT")
		set(WINDIR winnt)
	else()
		set(WINDIR win32)
	endif()
#libz
	add_library(z STATIC IMPORTED)
	set_target_properties(z PROPERTIES IMPORTED_LOCATION
		${CYANDEXDISK_SOURCE_DIR}/${WINDIR}/libz.a)
#pthreads
	find_package(Threads)
	if (NOT Threads_FOUND)
		add_library(pthread SHARED IMPORTED)
		set_target_properties(pthread PROPERTIES 
			IMPORTED_LOCATION
			${CYANDEXDISK_SOURCE_DIR}/${WINDIR}/pthreads-win32/lib/pthreadGC1.dll
			IMPORTED_IMPLIB ${CYANDEXDISK_SOURCE_DIR}/win32/pthreads-win32/lib/libpthreadGC1.a)	
		include_directories(${CYANDEXDISK_SOURCE_DIR}/win32/pthreads-win32/include)	
	endif()
	list(APPEND ADDLIBS pthread)
#curl
add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES 
		IMPORTED_LOCATION
		${CYANDEXDISK_SOURCE_DIR}/${WINDIR}/curl/lib/libcurl.dll.a)	
	  include_directories(${CYANDEXDISK_SOURCE_DIR}/win32/curl/include)
else()
	find_package(CURL)
	include_directories(${CURL_INCLUDE_DIRS})
endif()

add_library(${TARGET} STATIC 
	cYandexDisk.c 
	cYandexOAuth.c 
	cJSON.c 
	uuid4.c 
	${ADDSRC})

target_link_libraries(${TARGET} curl z ${ADDLIBS})

if(${BUILD_TEST})
	add_executable(cYandexDisk_test test.c)
	target_link_libraries(cYandexDisk_test ${TARGET})
endif()

#copy files
if (WIN32)
	#curl libs
	#Copy DLLs
	file(GLOB files 
		"${CYANDEXDISK_SOURCE_DIR}/${WINDIR}/curl/bin/*.dll"
		"${CYANDEXDISK_SOURCE_DIR}/${WINDIR}/curl/lib/*.dll"
		"${CYANDEXDISK_SOURCE_DIR}/${WINDIR}/pthreads-win32/lib/pthreadGC1.dll"
		)	
	#Copy
	foreach(_file ${files})
			file(INSTALL
				DESTINATION "${CMAKE_BINARY_DIR}"
				FILES "${_file}"
			)
	endforeach()	
endif()
