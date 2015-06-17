#include "recachesql.h"

/* For Windows, define PACKAGE_STRING in the VS project */
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
