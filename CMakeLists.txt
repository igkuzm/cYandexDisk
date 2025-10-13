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

# Determine the location of the libraries to use based on the platform.
if(ANDROID)
	ADD_LIBRARY(curl STATIC IMPORTED)
	SET_TARGET_PROPERTIES(curl PROPERTIES IMPORTED_LOCATION ${CYANDEXDISK_SOURCE_DIR}/curl/android/${ANDROID_ABI}/libcurl.a)
	INCLUDE_DIRECTORIES(${CYANDEXDISK_SOURCE_DIR}/curl/android/include)
	set(LIBZ z)
elseif(APPLE)
  if(IOS)
	ADD_LIBRARY(curl STATIC IMPORTED)
	SET_TARGET_PROPERTIES(curl PROPERTIES IMPORTED_LOCATION ${CYANDEXDISK_SOURCE_DIR}/curl/iOS/libcurl.a)
	INCLUDE_DIRECTORIES(${CYANDEXDISK_SOURCE_DIR}/curl/iOS/include)
  endif()
elseif(WIN32)
#libz
	add_library(z STATIC IMPORTED)
	set_target_properties(z PROPERTIES IMPORTED_LOCATION
		${CYANDEXDISK_SOURCE_DIR}/win32/libz.a)
#pthreads
	add_library(pthread SHARED IMPORTED)
	set_target_properties(pthread PROPERTIES IMPORTED_LOCATION
	${CYANDEXDISK_SOURCE_DIR}/win32/pthreads-win32/lib/pthreadGC1.dll)
	include_directories(${CYANDEXDISK_SOURCE_DIR}/win32/pthreads-win32/include)	
	set(ADDLIBS pthread)
	#add strptime
	#set(ADDSRC strptime.cc)
#curl
	add_library(curl SHARED IMPORTED)		
	set_target_properties(curl PROPERTIES IMPORTED_LOCATION
		${CYANDEXDISK_SOURCE_DIR}/win32/curl/bin/libcurl.dll)	
	include_directories(${CYANDEXDISK_SOURCE_DIR}/win32/curl/include)
endif()

if(APPLE)
	SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -framework Foundation")
endif()

add_library(${TARGET} STATIC 
	cYandexDisk.c 
	cYandexOAuth.c 
	cJSON.c 
	uuid4.c 
	${ADDSRC})

target_link_libraries(${TARGET} curl z ${ADDLIBS})
target_link_directories(${TARGET} PRIVATE ${ADDDIRS})

if(${CYD_BUILD_TEST})
	add_executable(cYandexDisk_test test.c)
	target_link_libraries(cYandexDisk_test ${TARGET})
endif()

#copy files
if (WIN32)
	#curl libs
	#Copy DLLs
	file(GLOB files 
		"${CYANDEXDISK_SOURCE_DIR}/win32/curl/bin/*.dll"
		"${CYANDEXDISK_SOURCE_DIR}/win32/pthreads-win32/lib/pthreadGC1.dll"
		)	
	#Copy
	foreach(_file ${files})
			file(INSTALL
				DESTINATION "${CMAKE_BINARY_DIR}"
				FILES "${_file}"
			)
	endforeach()	
endif()
