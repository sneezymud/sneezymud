#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <cctype>
#include <cassert>
#include <cstring>

#include <cerrno>
#include <string>
#include <vector>
#include <map>


static int gamePort=6968;

static int PROD_GAMEPORT=7900;
static int BUILDER_GAMEPORT=8900;

#define vlogf fprintf
#define LOG_DB stdout
#define FALSE 0
#define TRUE 1

#include "database.h"


static TDatabaseConnection database_connection;


TDatabase::TDatabase() : 
  res(NULL), 
  row(-1),
  db(NULL)
{
  //  vlogf(LOG_DB, "constructor");
}

TDatabase::TDatabase(string tdb) :
  res(NULL),
  row(-1),
  db(NULL)
{
  setDB(tdb);
  //    vlogf(LOG_DB, "constructor setDB");
}

TDatabase::~TDatabase(){
  PQclear(res);
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
// this is a little funky under postgres.  we initialize row to -1 when
// we do a query, then fetchRow increments it each time.
bool TDatabase::fetchRow(){
  if(!res)
    return FALSE;

  ++row;
  
  if(PQntuples(res)<=row)
    return FALSE;

  return TRUE;
}

// get one of the results from the current row of the current query
char *TDatabase::getColumn(int i){
  if(!res || row<0 || row >= PQntuples(res))
    return NULL;

  if(i > (PQnfields(res)-1) || i < 0){
    return NULL;
  } else {
    return PQgetvalue(res, row, i);
  }
}

// execute a query
bool TDatabase::query(const char *query,...){
  va_list ap;
  string buf;
  int fromlen=0, tolen=(512*2)+1, numlen=32;
  char *from=NULL, *fromptr, to[tolen], *toptr, numbuf[numlen];
  const char *qsave=query;
  PGresult *restmp;
  
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

	  // escaping the string needs a buffer that is 
	  // (string * 2) + 1 in size to avoid overruns
	  if(((fromlen*2)+1) > tolen){
	    vlogf(LOG_DB, "query - buffer overrun on %s", from);
	    vlogf(LOG_DB, "%s", qsave);
	    return FALSE;
	  }
	  
	  // escape ' and %
	  toptr=to;
	  fromptr=from;
	  while(*fromptr){
	    if(*fromptr == '\'' || *fromptr == '%'){
	      *toptr++='\\';
	    }
	    *toptr++=*fromptr++;
	  }
	  *toptr='\0';

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

  if(!(restmp=PQexec(db, buf.c_str()))){
    vlogf(LOG_DB, "query failed: %s", PQresStatus(PQresultStatus(restmp)));
    vlogf(LOG_DB, "%s", buf.c_str());
    return FALSE;
  }

  // if there is supposed to be some results from this query,
  // free the previous results (if any) and assign the new results
  // if there aren't supposed to be results (update, insert, delete, etc)
  // then don't do anything with the results, they might still be used
  if(restmp){
    if(res)
      PQclear(res);
    res=restmp;
    row=-1;
  }

  //  if(res)
  //    vlogf(LOG_DB, "New query results stored.");
  
  return TRUE;
}

bool TDatabase::isResults(){
  if(res && PQntuples(res))
    return TRUE;

  return FALSE;
}
