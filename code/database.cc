#include "stdsneezy.h"
#include "database.h"
#include "timing.h"


static TDatabaseConnection database_connection;

TDatabase::TDatabase() : 
  res(NULL), 
  row(NULL),
  db(NULL)
{
  row_count = 0;
  //  vlogf(LOG_DB, "constructor");
}

TDatabase::TDatabase(dbTypeT tdb) :
  res(NULL),
  row(NULL),
  db(NULL)
{
  setDB(tdb);
  row_count = 0;
  //  vlogf(LOG_DB, "constructor setDB");
}

TDatabase::~TDatabase(){
  mysql_free_result(res);
  //    vlogf(LOG_DB, "query results freed");
}

void TDatabase::setDB(dbTypeT tdb){
  switch(tdb){
    case DB_SNEEZY:
      db=database_connection.getSneezyDB();
      break;
    case DB_SNEEZYBETA:
      db=database_connection.getSneezyBetaDB();
      break;
    case DB_IMMORTAL:
      db=database_connection.getImmoDB();
      break;
    case DB_SNEEZYGLOBAL:
      db=database_connection.getSneezyGlobalDB();
      break;
    case DB_SNEEZYPROD:
      db=database_connection.getSneezyProdDB();
      break;
    default:
      vlogf(LOG_DB, fmt("Unknown database dbTypeT %i") %  tdb);
      db=NULL;
  }
}

// advance to the next row of the current query
// this is a little funky under postgres.  we initialize row to -1 when
// we do a query, then fetchRow increments it each time.
bool TDatabase::fetchRow(){
  if(!res)
    return FALSE;
  
  if(!(row=mysql_fetch_row(res)))
    return FALSE;
  
  return TRUE;
}

const sstring TDatabase::operator[] (const sstring &s) const
{
  if(!res || !row)
    return NULL;

  unsigned int num_fields=mysql_num_fields(res);
  MYSQL_FIELD *fields=mysql_fetch_fields(res);
  unsigned int i;
  sstring fieldname;

  for(i=0;i<num_fields;++i){
    fieldname=fields[i].name;
    if(s.lower()==fieldname.lower()){
      break;
    }
  }

  if(i > (mysql_num_fields(res)-1) || i < 0){
    vlogf(LOG_DB, fmt("TDatabase::operator[%s] - invalid column name") %  s);
    return empty;
  } else {
    return row[i];
  }
}


// execute a query
bool TDatabase::query(const char *query,...)
{
  va_list ap;
  sstring buf;
  int fromlen=0, tolen=(32768*2)+1;
  const char *qsave=query;
  char *from=NULL, to[tolen];
  MYSQL_RES *restmp;
  TTiming t;

  t.start();

  // no db set yet
  if(!db)
    return FALSE;

  va_start(ap, query);
  do {
    if(*query=='%'){
      query++;
      switch(*query){
	case 'r':
	  // this is just like %s, but it doesn't do escaping
	  // use with caution!  should not be used with user input
	  from=va_arg(ap, char *);
	 	  	  
	  if(!from){
	    vlogf(LOG_DB, "null argument for format specifier 'r'");
	    vlogf(LOG_DB, fmt("%s") % qsave);	    
	  }
	  
	  buf += from;
	  break;
	case 's':
	  from=va_arg(ap, char *);
	 	  	  
	  if(!from){
	    vlogf(LOG_DB, "null argument for format specifier 's'");
	    vlogf(LOG_DB, fmt("%s") % qsave);	    
	  }

	  fromlen=strlen(from);
	  
	  // mysql_escape_string needs a buffer that is 
	  // (string * 2) + 1 in size to avoid overruns
	  if(((fromlen*2)+1) > tolen){
	    vlogf(LOG_DB, fmt("query - buffer overrun on %s") % from);
	    vlogf(LOG_DB, fmt("%s") % qsave);
	    return FALSE;
	  }
	  
	  mysql_escape_string(to, from, strlen(from));
	  buf += to;
	  break;
	case 'i':
	  buf = fmt("%s%i") % buf % va_arg(ap, int);
	  break;
	case 'f':
	  buf = fmt("%s%f") % buf % va_arg(ap, double);
	  break;
	case '%':
	  buf += "%";
	  break;
	default:
	  vlogf(LOG_DB, fmt("query - bad format specifier - %c") %  *query);
	  vlogf(LOG_DB, fmt("%s") %  qsave);
	  return FALSE;
      }
    } else {
      buf += *query;
    }
  } while(*query++);
  va_end(ap);

  if(mysql_query(db, buf.c_str())){
    vlogf(LOG_DB, fmt("query failed: %s") % mysql_error(db));
    vlogf(LOG_DB, fmt("%s") % buf);
    return FALSE;
  }
  restmp=mysql_store_result(db);

  // if there is supposed to be some results from this query,
  // free the previous results (if any) and assign the new results
  // if there aren't supposed to be results (update, insert, delete, etc)
  // then don't do anything with the results, they might still be used
  if(restmp){
    if(res)
      mysql_free_result(res);
    res=restmp;
  }
  
  // capture rowcount here, because the db pointer state changes when db timing is on
  row_count = (long) mysql_affected_rows(db);

  t.end();

  if(t.getElapsed() > 1.0){
    vlogf(LOG_DB, fmt("Query took %f seconds.") % t.getElapsed());
    vlogf(LOG_DB, fmt("%s") % buf);
  }

  // this saves the queries (without args) and the execution time
  // it slows things down pretty significantly though
  if(toggleInfo.isLoaded() && toggleInfo[TOG_DBTIMING]->toggle){
    query=qsave;
    buf="";

    // escape ' and %
    while(*query){
      if(*query == '\'' || *query == '%'){
	buf += "\\";
      }
      buf += *query++;
    }

    buf = fmt("insert into querytimes (query, secs) values ('%s', %f)") % 
      buf % t.getElapsed();

    mysql_query(db, buf.c_str());
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

long TDatabase::rowCount(){
  // return # of affected or retrieved rows
  // -1 if query returned an error
  
  // this gets set in TDatabase::query
  // because the db pointer will have changed state if query timing is on
  return row_count;
}
