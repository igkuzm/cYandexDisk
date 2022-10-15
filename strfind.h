/**
 * File              : strfind.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.02.2022
 * Last Modified Date: 15.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/**Usage:  
 *to replace all matches:
 *		  char * new_string = strarep(old_string, search_pattern, replace_string);
 *to replace from string position:
 *		  char * new_string = strrep(old_string, position, length, replace_string);
 *to find position of first match:
 *		  int pos = strfind(string, search_pattern)
 *to count matches:
 *		  int count = strcnt(string, search_pattern)
 */


#ifndef k_lib_strfnd_h__
#define k_lib_strfnd_h__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//check haystack does starts with and include needle - return position of pointer haystack + needle_len or NULL
static char * strinc(const char * haystack, const char * needle);

//find needle in haystack and return its position 
static long strfnd(const char * haystack, const char * needle);
#define strfind(haystack, needle) strfnd(haystack, needle)

//return count of needle matches in haystack
static int strcnt(const char * haystack, const char * needle);

//return new allocated string with contents of haystack replaced from position to position+len by replace string  
static char * strrep(const char * haystack, size_t pos, size_t len, const char * replace);

//return new allocated string with contents of haystack and inserted needle in pos position 
static char * strins(const char *haystack, const char *needle, int pos);

//return new allocated string with contents of haystack replaced all needle matches by replace string
static char * strarep(const char * haystack, const char * needle, const char * replace);

//create out file copy of in file with all needle replaced by replace and return it's count 
static int strfrep(FILE *in, FILE *out, const char * needle, const char * replace); 

/**
 * implimation of functions
 */

char * 
strinc(
		const char * haystack, 
		const char * needle
		)
{
	char * hp = (char *)haystack; //haystack pointer
	char * np = (char *)needle;   //needle pointer

	while(*np)
		if (*hp++ != *np++)
			return NULL;
	
	return hp++; //return pointer next to needle len
}


long 
strfnd( 
		const char * haystack, 
		const char * needle
		)
{
	//find position of search word in haystack
	const char *p = strstr(haystack, needle);
	if (p)
		return p - haystack;
	return -1;
}

int
strcnt(
		const char * haystack,   //string to scan 
		const char * needle		 //search pattern
		)
{
	char *ptr = (char *)haystack; //haystack pointer
	int i = 0, cnt = 0;
	while (*ptr) {
		char *p = strinc(ptr, needle); 
		if (p){
			cnt++; ptr = p;
		}
		i++; ptr++;
	}

	return cnt;
}

char * 
strrep(
		const char * haystack,       //string
		size_t pos,		             //start position
		size_t len,					 //length of replace string
		const char * replace	     //replace string
		)
{
	//find len of string
	size_t stringlen = strlen(haystack);
	size_t replacelen = strlen(replace);

	//allocate result string with enough memory
	char * result = (char *)malloc(stringlen - len + replacelen + 1);
	if (result == NULL) {
		perror("result string malloc");
		exit(EXIT_FAILURE);
	}	

	//replace string
	int i = 0;
	while (*haystack) {
		if (i == pos) { //if found word
			strncat(result, replace, replacelen); //copy new string to result
			i += replacelen; //move result counter
			haystack += len; //move read string counter
		} else {
			result[i++] = *haystack++;
		}
	}
	result[i] = '\0';
	return result;
}

char * strins(const char *haystack, const char *needle, int pos){
	char * hp = (char *)haystack; //haystack pointer
	char * np = (char *)needle;   //needle pointer

	char *str = (char *)malloc(strlen(haystack) + strlen(needle) + 1); //allocate enough memory
	if (str == NULL) {
		perror("str malloc");
		exit(EXIT_FAILURE);
	}
		
	int i = 0;
	while (*hp) {
		if (i == pos) {
			while(*np){
				str[i] = *np;
				i++; np++;
			}
		}
		str[i] = *hp;
		i++; hp++;
	}
	str[i++] = 0;

	return str;
}

char * //return result string
strarep(
		const char * haystack,       //string to scan 
		const char * needle,		 //search pattern
		const char * replace		 //replace string
	){

	size_t needlelen = strlen(needle);
	size_t replacelen = strlen(replace);

	//count cases of search word in haystack
	int i, cnt = 0;
	for (i = 0; haystack[i] != '\0'; i++) {
		if (strinc(haystack, needle)) {
			cnt++;

			// Jumping to index after the needle word.
			i += needlelen - 1;
		}
	}

	//allocate result string with enough memory
	char *result = (char *)malloc(i + cnt * (replacelen - needlelen) + 1);
	if (result == NULL) {
		perror("result string malloc");
		exit(EXIT_FAILURE);
	}

	//replace search needle string with replace string
	char *ptr = (char *)haystack; //haystack pointer
	i = 0;
	while (*ptr) {
		char *p = strinc(ptr, needle); 
		if (p){
			strncat(result, replace, replacelen);
			ptr = p;
		} else {
			result[i++] = *ptr++;
		}
	}
	result[i] = '\0';
	return result;
}

int 
strfrep(
		FILE *in, 
		FILE *out, 
		const char * needle, 
		const char * replace 
		) 

{
	int nlen=strlen(needle);
	int rlen=strlen(replace);

	char buff[nlen];
	
	int i, cnt = 0;

    while (1) { 
        char ch = fgetc(in);
        if (ch == EOF) { 
            break; 
        }
		
		if (ch==needle[i]) {
			buff[i]=ch;
			i++;
			if (i==nlen) {
				//found string matches
				cnt++;
				
				//replace string
				for (i = 0; i < rlen; ++i) {
					fputc(replace[i], out);
				}
				i=0;
			}
		}
		else {
			if (i>0) {
				//print buff	
				int c=i;
				for (i = 0; i < c; ++i) {
					fputc(buff[i], out);
				}
			}
			i=0;
			fputc(ch,out);
		}
	}

	return cnt;
}



#ifdef __cplusplus
}
#endif

#endif //k_lib_strfnd_h__
