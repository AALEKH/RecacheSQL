#include "recachesql.h"

/* For Redis Operation */

#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "hiredis/adapters/ae.h"

/**
 * Allocate unused space for a string with a specific size and null terminate it.
 * The string does not need to be null terminated.
 * For Windows, define PACKAGE_STRING in the VS project */

#ifndef __WIN__
#include "config.h"
#endif

/* These must be right or mysqld will not find the symbol! */
#ifdef	__cplusplus
extern "C" {
#endif
	DLLEXP my_bool recachesql_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
	DLLEXP void recachesql_info_deinit(UDF_INIT *initid);
	/* For functions that return STRING or DECIMAL */ 
	DLLEXP char *recachesql_info(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

	/* For functions that return REAL */
	/* DLLEXP double recachesql_info(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error); */
	/* For functions that return INTEGER */
	/* DLLEXP longlong recachesql_info(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error); */

	/* If you are doing an Aggregate function you'll need these too */
	/* DLLEXP void recachesql_info_clear( UDF_INIT* initid, char* is_null, char* is_error ); */
	/* DLLEXP void recachesql_info_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error ); */

#ifdef	__cplusplus
}
#endif


/*
 * Output the library version.
 * recachesql_info()
 */

my_bool recachesql_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	return 0;
}

void recachesql_info_deinit(UDF_INIT *initid)
{
}

/* For functions that return REAL */
/* double recachesql_info(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) */
/* For functions that return INTEGER */
/* longlong recachesql_info(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) */

/* For functions that return STRING or DECIMAL */ 
char* recachesql_info(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length,	char *is_null, char *error)
{
	strcpy(result, PACKAGE_STRING);
	*length = strlen(PACKAGE_STRING);
	return result;
}

char *strncpy_alloc(const char *str, unsigned long length)
{
	if (str == NULL) return NULL;

	char *newstr = (char *)malloc((length+1) * sizeof(char));
	if (newstr == NULL) return NULL;

	strncpy(newstr, str, length);
	newstr[length] = '\0';

	return newstr;
}

/**
 * Allocate unused space for an array of nelem elements each of whose size in bytes is elsize.
 * The space shall be initialized to all bits 0.
 */
void **ptr_calloc(size_t nelem, size_t elsize)
{
	void **ptr = (void **)malloc(nelem * elsize + sizeof(int));
	if (ptr == NULL) return NULL;

	*(int *)ptr = nelem;

	ptr = (void **)((int*)ptr + 1);
	memset(ptr, 0, nelem * elsize);

	return ptr;
}

/**
 * Free allocated space of ptr and the space of all items from ptr.
 * Only use with mem allocated with ptr_calloc.
 */
void ptr_free(void **ptr)
{
    int i;
	for (i=0; i < *((int *)ptr - 1); i++) {
		if (ptr[i]) free(ptr[i]);
	}

	free((int*)ptr-1);
}

/**
 * Compare 2 (not \0 term) strings case insensative, specifying the length
 */
int strncmp_caseins(char *str1, char *str2, size_t num)
{
	char c1, c2;
    int i;

	for (i=0; i<num; i++) {
		c1 = (str1[i] >= 65 && str1[i] <= 90) ? str1[i] + 32 : str1[i]; /* Change to lower case */
		c2 = (str2[i] >= 65 && str2[i] <= 90) ? str2[i] + 32 : str2[i]; /* Change to lower case */

		if (c1 != c2) return (c1 < c2) * -2 + 1;   /* Could have used q?a:b, but... nerd power */
	}

	return 0;
}

/**
 * Check if a char appears in a string, specifying the length
 */
int charinstr(char *str, char c, size_t num)
{
    int i;

	for (i=0; i<num && str[i]; i++) {
		if (str[i] == c) return i;
	}

	return -1;
}

/**
 * Copy an attribute name, skipping backquotes and everything before a dot
 */
char *copy_argname(char *att, unsigned long length)
{
	char *attcl = att;
	char *str, *ptr;
	char quoting = 0;

	for (ptr=att; ptr<att+length; ptr++) {
		if (*ptr == '`') quoting != quoting;
		 else if (!quoting && *ptr == '.') attcl = ptr+1;
	}

	length = length - (attcl-att);

	if (!quoting) {
		if (attcl[0] == '`') { attcl++; length--; }
		if (*(attcl+length-1) == '`') length--;
	}

	str = (char *)malloc(length + 1);
	if (!str) return NULL;

	strncpy(str, attcl, length);
	str[length] = 0;

	return str;
}

my_bool recachesql_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	if (args->arg_count != 1) {
       		strcpy(message, "recachesql can only accept one argument");
		return 1; 
	}
	
	if (args->arg_type[0] != STRING_RESULT) {
		strcpy(message, "recachesql argument has to be an string");
		return 1; 
	}
	
	return 0;
}

int recachesql(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
	redisContext *c;
	redisReply *reply;
	c = redisConnect("127.0.0.1", 6379);
	if(c!=NULL && c->err){
		printf("Error: %s\n", c->errstr);
	}
	reply = redisCommand(c,"SET %s %s", "foo", "hello world");
	printf("SET: %s\n", reply->str);
	freeReplyObject(reply);

	reply = redisCommand(c,"GET foo");
	printf("GET foo: %s\n", reply->str);
	freeReplyObject(reply);
	redisFree(c);
	return 0;
}
