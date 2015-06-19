#ifndef RECACHESQL_H_
#define RECACHESQL_H_

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(WIN32)
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#include <m_string.h>
#endif
#include <mysql.h>
#include <ctype.h>

//#include "config.h"

char *strncpy_alloc(const char *str, unsigned long length);
void **ptr_calloc(size_t nelem, size_t elsize);
void ptr_free(void **ptr);
int strncmp_caseins(char *str1, char *str2, size_t num);
int charinstr(char *str, char c, size_t num);
char *copy_argname(char *att, unsigned long length);

#define TRIM_BACKQUOTE(fnull) (fnull+(int)(fnull[0]=='`'))		// Skip starting backquote
#define RETURN_ERR(msg) { strcpy(message, msg); return 1; }		// Set error message and return in %_init functions
#define ATTRIBUTE_COMPARE(i, str, len) (args->attribute_lengths[i] == len && strncmp_caseins(args->attributes[i], str, len) == 0)

#endif
