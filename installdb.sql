USE mysql;

DROP FUNCTION IF EXISTS recachesql_info;
CREATE FUNCTION recachesql_info RETURNS STRING SONAME 'recachesql.so';
