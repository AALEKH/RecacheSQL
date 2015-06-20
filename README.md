RecacheSQL
===========

**Redis UDF to insert column values in a MariaDB/MySQL table to Redis list **

This UDF provide functionality to insert values from MariaDB/MySQL table to Redis. Current supported data structure for Redis is list, but other support are possible in future. **Please do not use this in production right now**, as this is not very well tested and **crashes the MariaDB/MySQL server at many times**. 
   
## Dependencies:
 
  * Redis(http://redis.io)
  * libtool(http://apr.apache.org/download.cgi)
  * Development tool as per your enviornment (libmysqlclient(https://packages.debian.org/wheezy/libmysqlclient-dev)      in case of MySQL and libmariadbclient-dev(http://www.howtoinstall.co/en/ubuntu/trusty/universe/libmariadbclient-d     ev/) in case of MariaDB)
  * GCC

## To compile the UDF's run:

* For MySQL: **gcc -o recachesql.so recachesql.c `/path/to/database-server/mysql_config --cflags` -lmysqlclient -lhiredis -levent -bundle -Wl,-undefined -Wl,dynamic_lookup**
* For MariaDB: **gcc -o recachesql.so recachesql.c `/path/to/database-server/mysql_config --cflags` -lmariadbclient -lhiredis -levent -bundle -Wl,-undefined -Wl,dynamic_lookup**
* And move the resulting recachesql.so to /lib/plugin/ folder of the database-server

## To Install run:

* CREATE FUNCTION recachesql RETURNS int SONAME 'recachesql.so';

## Usage:

* SELECT recachesql(database, table-name);

## Example:

* Select recachesql("testdb", "Cars"), will store all teh values existing in a column as Redis-list. 
* To access the value name in that UDF simply enter (in redis-cli): lget `TABLE NAME-COLUMN NAME`



