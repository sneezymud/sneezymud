#include "stdsneezy.h"
#include "database.h"
#include "timing.h"


static TDatabaseConnection database_connection;

TDatabase::TDatabase() : 
  res(NULL), 
  row(-1),
  db(NULL)
{
  //  vlogf(LOG_DB, "constructor");
}

TDatabase::TDatabase(dbTypeT tdb) :
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
    case DB_SNEEZYBUILDER:
      db=database_connection.getSneezyBuilderDB();
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

  ++row;
  
  if(PQntuples(res)<=row)
    return FALSE;

  return TRUE;
}

const sstring TDatabase::operator[] (const sstring &s) const
{
  if(!res || row<0 || row >= PQntuples(res))
    return NULL;
  
  int i=PQfnumber(res, s.c_str());

  if(i < 0){
    vlogf(LOG_DB, fmt("TDatabase::operator[%s] - invalid column name") %  s);
    return empty;
  } else {
    return PQgetvalue(res, row, i);
  }
}


// execute a query
bool TDatabase::query(const char *query,...)
{
  va_list ap;
  sstring buf;
  const char *qsave=query;
  char *from=NULL;
  PGresult *restmp;
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
	case 's':
	  from=va_arg(ap, char *);
	  
	  // escape ', % and backslash
	  while(*from){
	    if(*from == '\'' || *from == '%' || *from == '\\'){
	      buf += "\\";
	    }
	    buf += *from++;
	  }

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

  if(!(restmp=PQexec(db, buf.c_str())) ||
     (PQresultStatus(restmp) != PGRES_COMMAND_OK &&
      PQresultStatus(restmp) != PGRES_TUPLES_OK)){
    vlogf(LOG_DB, fmt("query failed: %s") %  PQresStatus(PQresultStatus(restmp)));
    vlogf(LOG_DB, fmt("%s") %  buf);
    return FALSE;
  }

  // if there is supposed to be some results from this query,
  // free the previous results (if any) and assign the new results
  // if there aren't supposed to be results (update, insert, delete, etc)
  // then don't do anything with the results, they might still be used
  if(restmp){
    if(res)
      PQclear(res); // free the old results
    res=restmp;     // assign the new results
    row=-1;         // "rewind" the row count
  }

  t.end();

  // this saves the queries (without args) and the execution time
  // it slows things down pretty significantly though
  if(timeQueries){
    query=qsave;
    buf="";

    // escape ' and %
    while(*query){
      if(*query == '\'' || *query == '%'){
	buf += "\\";
      }
      buf += *query++;
    }

    buf = fmt("insert into querytimes values ('%s', %f)") % 
      buf % t.getElapsed();
    
    PQexec(db, buf.c_str());
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
