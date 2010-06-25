#include <stdarg.h>

#include "configuration.h"
#include "extern.h"
#include "database.h"
#include "timing.h"
#include "toggle.h"

TDatabaseConnection database_connection;

// we return this instead of null if they try to fetch an invalid column
const sstring empty="";

std::vector <std::string> db_hosts(DB_MAX);

const char * db_connect[DB_MAX] = {
  NULL, // depends on game port
  "sneezybeta",
  "immortal",
  "sneezyglobal",
  "sneezy", 
  "sneezybuilder",
  "wikidb",
  "builder_wikidb",
  "mudadmin_wikidb",
  "smf",
  };


TDatabaseConnection::TDatabaseConnection()
{
  memset(databases, 0, sizeof(databases));
}


const char *TDatabaseConnection::getConnectParam(dbTypeT type)
{
  const char *ret = db_connect[type];
  if (ret)
    return ret;
  if (gamePort == Config::Port::PROD)
    return db_connect[DB_SNEEZYPROD];
  if (gamePort == Config::Port::BUILDER)
    return db_connect[DB_SNEEZYBUILDER];
  return db_connect[DB_SNEEZYBETA];
}


MYSQL *TDatabaseConnection::getDB(dbTypeT type)
{
  if (type < 0 || type > DB_MAX)
    return NULL;

  if (!databases[type] || mysql_ping(databases[type]))
  {
    vlogf(LOG_DB, format("Initializing database '%s'.") % getConnectParam(type));
    databases[type] = mysql_init(NULL);
    
    vlogf(LOG_DB, "Connecting to database.");
    if(!mysql_real_connect(databases[type], db_hosts[type].c_str(), "sneezy", NULL, getConnectParam(type), 0, NULL, 0))
    {
      vlogf(LOG_DB, format("Could not connect to database '%s'.") % getConnectParam(type));
      vlogf(LOG_DB, format("%s") % mysql_error(databases[type]));
      return NULL;
    }
  }
  return databases[type];
}


TDatabase::TDatabase() : 
  res(NULL), 
  row(NULL),
  db(NULL)
{
  row_count = 0;
}

TDatabase::TDatabase(dbTypeT tdb) :
  res(NULL),
  row(NULL),
  db(NULL)
{
  setDB(tdb);
  row_count = 0;
}

TDatabase::~TDatabase(){
  mysql_free_result(res);
}

void TDatabase::setDB(dbTypeT tdb){
  db = database_connection.getDB(tdb);
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

const sstring TDatabase::operator[] (unsigned int i) const
{
  if(res && i > (mysql_num_fields(res)-1)){
    vlogf(LOG_DB, format("TDatabase::operator[%i] - invalid column index") % i);
    return empty;
  } else {
    return row[i];
  }
}

const sstring TDatabase::operator[] (const sstring &s) const
{
  if(!res || !row)
    return NULL;

  std::map<const char*, int, ltstr>::const_iterator cur=column_names.find(s.lower().c_str());

  if(cur == column_names.end()){
    vlogf(LOG_DB, format("TDatabase::operator[%s] - invalid column name") %  s);
    return empty;
  }

  return row[(*cur).second];
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
	    vlogf(LOG_DB, format("%s") % qsave);	    
	  }
	  
	  buf += from;
	  break;
	case 's':
	  from=va_arg(ap, char *);
	 	  	  
	  if(!from){
	    vlogf(LOG_DB, "null argument for format specifier 's'");
	    vlogf(LOG_DB, format("%s") % qsave);	    
	  }

	  fromlen=strlen(from);
	  
	  // mysql_escape_string needs a buffer that is 
	  // (string * 2) + 1 in size to avoid overruns
	  if(((fromlen*2)+1) > tolen){
	    vlogf(LOG_DB, format("query - buffer overrun on %s") % from);
	    vlogf(LOG_DB, format("%s") % qsave);
	    return FALSE;
	  }
	  
	  mysql_escape_string(to, from, strlen(from));
	  buf += to;
	  break;
	case 'i':
	  buf = format("%s%i") % buf % va_arg(ap, int);
	  break;
	case 'f':
	  buf = format("%s%f") % buf % va_arg(ap, double);
	  break;
	case '%':
	  buf += "%";
	  break;
	default:
	  vlogf(LOG_DB, format("query - bad format specifier - %c") %  *query);
	  vlogf(LOG_DB, format("%s") %  qsave);
	  return FALSE;
      }
    } else {
      buf += *query;
    }
  } while(*query++);
  va_end(ap);

  if(mysql_query(db, buf.c_str())){
    vlogf(LOG_DB, format("query failed: %s") % mysql_error(db));
    vlogf(LOG_DB, format("%s") % buf);
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
  
  // capture rowcount here, because the db pointer state changes when
  // db timing is on
  row_count = (long) mysql_affected_rows(db);

  // store the column names and offsets
  if(res){
    column_names.clear();
    unsigned int num_fields=mysql_num_fields(res);
    MYSQL_FIELD *fields=mysql_fetch_fields(res);
    
    for(unsigned int i=0;i<num_fields;++i)
      column_names[fields[i].name]=i;
  }


  t.end();

  if(t.getElapsed() > 1.0){
    vlogf(LOG_DB, format("Query took %f seconds.") % t.getElapsed());
    vlogf(LOG_DB, format("%s") % buf);
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

    buf = format("insert into querytimes (query, secs) values ('%s', %f)") % 
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
