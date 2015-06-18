#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "hiredis/adapters/libevent.h"

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

void concat(char s1[], char s2[]) {
   int i, j;
 
   i = strlen(s1);
 
   for (j = 0; s2[j] != '\0'; i++, j++) {
      s1[i] = s2[j];
   }
 
   s1[i] = '\0';
}

void get_values(MYSQL *con, char row[20], char t_name[25] ) {
  
  redisContext *c;
  redisReply *reply;
  char vi[1024] = "SELECT ";
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
      for(int i,j = 0; i < num_fields; i++, j++) 
      { 
        concat(row, sprintf(str, "%d", j));
        printf("see this: %s\n", t_name);
        reply = redisCommand(c,"HMSET %s %s %s", t_name, row, row_name[i]);
        printf("%s\n", reply);
        freeReplyObject(reply);
        //printf("%s ", row_name[i] ? row_name[i] : "NULL"); HMSET tutorialspoint name-aa "aalekh" 
      } 
          printf("\n\n\n\n"); 
  }

}

int main(int argc, char **argv)
{      
  MYSQL *con = mysql_init(NULL);
  
  if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
      exit(1);
  }  
  
  if (mysql_real_connect(con, "localhost", "root", "", "testdb", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }
  //SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = 'testdb' AND TABLE_NAME = 'Cars';

  char a[1024] = "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '";
  concat(a, "testdb");
  concat(a, "' AND TABLE_NAME = '");
  concat(a, "Cars");
  concat(a, "'");
  if (mysql_query(con, a)) 
  {
      finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  
  if (result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  
  while ((row = mysql_fetch_row(result))) 
  { 
      for(int i = 0; i < num_fields; i++) 
      { 
          //printf("%s ", row[i] ? row[i] : "NULL");
          get_values(con, row[i], "Cars"); 
      } 
          printf("\n"); 
  }
  
  mysql_free_result(result);
  mysql_close(con);
  
  exit(0);
}