#include "stdsneezy.h"
#include "database.h"

static TDatabaseConnection database_connection;


TDatabase::TDatabase() : 
  res(NULL), 
  row(NULL),
  db(NULL)
{
  //  vlogf(LOG_DB, "constructor");
}

TDatabase::TDatabase(string tdb) :
  res(NULL),
  row(NULL),
  db(NULL)
{
  setDB(tdb);
  //    vlogf(LOG_DB, "constructor setDB");
}

TDatabase::~TDatabase(){
  mysql_free_result(res);
  //    vlogf(LOG_DB, "query results freed");
}

void TDatabase::setDB(string tdb){
  if(tdb=="sneezy"){
    db=database_connection.getSneezyDB();
  } else if(tdb=="immortal"){
    db=database_connection.getImmoDB();
  } else {
    vlogf(LOG_DB, "Unknown database %s", tdb.c_str());
    db=NULL;
  }
}

// advance to the next row of the current query
bool TDatabase::fetchRow(){
  if(!res)
    return FALSE;
  
  if(!(row=mysql_fetch_row(res)))
    return FALSE;
  
  return TRUE;
}

// get one of the results from the current row of the current query
char *TDatabase::getColumn(unsigned int i){
  if(!res || !row)
    return NULL;

  if(i > (mysql_num_fields(res)-1) || i < 0){
    return NULL;
  } else {
    return row[i];
  }
}

// execute a query
bool TDatabase::query(const char *query,...){
  va_list ap;
  string buf;
  int fromlen=0, tolen=(512*2)+1, numlen=32;
  char *from=NULL, to[tolen], numbuf[numlen];
  const char *qsave=query;
  MYSQL_RES *restmp;
  
  // no db set yet
  if(!db)
    return FALSE;

  va_start(ap, query);
  do {
    if(*query=='%'){
      query++;
      switch(*query){
	case 's':
	  from=va_arg(ap, char *);
	  
	  if(!from){
	    vlogf(LOG_DB, "null argument for format specifier 's'");
	    vlogf(LOG_DB, "%s", qsave);	    
	  }

	  fromlen=strlen(from);
	  
	  // mysql_escape_string needs a buffer that is 
	  // (string * 2) + 1 in size to avoid overruns
	  if(((fromlen*2)+1) > tolen){
	    vlogf(LOG_DB, "query - buffer overrun on %s", from);
	    vlogf(LOG_DB, "%s", qsave);
	    return FALSE;
	  }
	  
	  mysql_escape_string(to, from, strlen(from));
	  buf += to;
	  break;
	case 'i':
	  snprintf(numbuf, numlen-1, "%i", va_arg(ap, int));
	  buf += numbuf;
	  break;
	case 'f':
	  snprintf(numbuf, numlen-1, "%f", va_arg(ap, double));
	  buf += numbuf;
	  break;
	case '%':
	  buf += "%";
	  break;
	default:
	  vlogf(LOG_DB, "query - bad format specifier - %c", *query);
	  vlogf(LOG_DB, "%s", qsave);
	  return FALSE;
      }
    } else {
      buf += *query;
    }
  } while(*query++);
  va_end(ap);
  
  if(mysql_query(db, buf.c_str())){
    vlogf(LOG_DB, "query failed: %s", mysql_error(db));
    vlogf(LOG_DB, "%s", buf.c_str());
    return FALSE;
  }
  restmp=mysql_store_result(db);

  // if there is supposed to be some results from this query,
  // free the previous results (if any) and assign the new results
  // if there aren't supposed to be results (update, insert, delete, etc)
  // then don't do anything with the results, they might still be used
  if(mysql_field_count(db)){
    if(res)
      mysql_free_result(res);
    res=restmp;
  }

  //  if(res)
  //    vlogf(LOG_DB, "New query results stored.");
  
  return TRUE;
}

bool TDatabase::isResults(){
  if(res && mysql_num_rows(res))
    return TRUE;

  return FALSE;
}
