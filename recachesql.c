#include "recachesql.h"

/* Generic C Header's */
#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>

/* For Redis Operation */
#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "hiredis/adapters/libevent.h"

/**
 * Allocate unused space for a string with a specific size and null terminate it.
 * The string does not need to be null terminated.
 * For Windows, define PACKAGE_STRING in the VS project */

#ifndef __WIN__
//#include "config.h"
#endif

/*w These must be right or mysqld will not find the symbol! */
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

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

/*
* Simple String Concatenation Function
*/
void concat(char s1[], char s2[]) {
   int i, j;
 
   i = strlen(s1);
 
   for (j = 0; s2[j] != '\0'; i++, j++) {
      s1[i] = s2[j];
   }
 
   s1[i] = '\0';
}

/* 
* Function to be used for construct Redis List from MySQL Table Column's 
*/
void get_values(MYSQL *con, char row[200], char t_name[250] ) {
  
  redisContext *c;
  redisReply *reply;
  char vi[1024] = "SELECT ";
  int j = 0;
  char str[500];
  c = redisConnect("127.0.0.1", 6379);
  if(c!=NULL && c->err){
    printf("Error: %s\n", c->errstr);
  }
  concat(vi, row);
  concat(vi, " from ");
  concat(vi, t_name);
  printf("%s\n", row);
  if (mysql_query(con, vi)) 
  {
      finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  if (result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row_name;
  
  while ((row_name = mysql_fetch_row(result))) 
  {  
      for(int i = 0; i < num_fields;) 
      { 
        j = j + 1;
        redisCommand(c,"LPUSH %s-%s '%s'", t_name, row, row_name[i]);
        i = i + 1;
      } 
  }

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

	MYSQL *con = mysql_init(NULL);

	if (con == NULL) {

	    fprintf(stderr, "mysql_init() failed\n");
	    exit(1);
	}  
	  
	if (mysql_real_connect(con, "localhost", "root", "", args->args[0], 0, NULL, 0) == NULL) {

	    finish_with_error(con);
	}
	//SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = 'testdb' AND TABLE_NAME = 'Cars';

	char a[1024] = "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '";
	concat(a, args->args[0]);
	concat(a, "' AND TABLE_NAME = '");
	concat(a, args->args[1]);
	concat(a, "'");
	if (mysql_query(con, a)) {

	      finish_with_error(con);
	}
	  
	MYSQL_RES *result = mysql_store_result(con);
	  
	if (result == NULL) {

	    finish_with_error(con);
	}

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	  
	while ((row = mysql_fetch_row(result))) {

	    for(int i = 0; i < num_fields; i++) {

	        get_values(con, row[i], args->args[1]); 
	    } 
	        printf("\n"); 
	}
	  
	mysql_free_result(result);
	mysql_close(con);
	  
	exit(0);
	return 0;
}

my_bool rcdecachesql_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{

     if (args->arg_count != 2)
     {
       strcpy(message, "recachesql udf's takes 2 arguments");
	   return 1; 
	}

     return 0;
}

void recachesql_deinit(UDF_INIT *initid)
{
	free(initid->ptr);
}

