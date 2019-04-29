#include <cassert>
#include <stdarg.h>
#include <mariadb/mysql.h>

#include "configuration.h"
#include "extern.h"
#include "database.h"
#include "timing.h"
#include "toggle.h"

namespace {
    struct ltstr
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return strcmp(s1, s2) < 0;
        }
    };


    // we return this instead of null if they try to fetch an invalid column
    const sstring empty="";

    const char * db_connect[DB_MAX] = {
        "sneezy",
        "immortal",
    };

    // maintain instances of sneezydb and immodb
    class TDatabaseConnection
    {
        MYSQL *databases[DB_MAX];
        public:
        TDatabaseConnection();

        const char *getConnectParam(dbTypeT type);
        MYSQL *getDB(dbTypeT type);

        void clearConnections(){ for(int i=0;i<DB_MAX;++i) databases[i]=NULL; }
    };

    TDatabaseConnection::TDatabaseConnection()
    {
        memset(databases, 0, sizeof(databases));
    }

    TDatabaseConnection database_connection;
}

std::vector <std::string> db_hosts(DB_MAX);
std::vector <std::string> db_names(DB_MAX);
std::vector <std::string> db_users(DB_MAX);
std::vector <std::string> db_passwords(DB_MAX);


const char *TDatabaseConnection::getConnectParam(dbTypeT type)
{
  assert(type >= 0);
  assert(type < DB_MAX);
  if (db_names[type] != "")
    return db_names[type].c_str();
  return db_connect[type];
}

class TDatabasePimpl {
public:
  MYSQL_RES *res;
  MYSQL_ROW row;
  MYSQL *db;
  long row_count;
  bool log;
  std::map <const char *, int, ltstr> column_names;

  TDatabasePimpl() :
    res(NULL),
    row(NULL),
    db(NULL)
  {
  }
};

static const char* getUser(dbTypeT type)
{
  if (db_users[type] != "")
    return db_users[type].c_str();
  else
    return NULL;
}


static const char* getPass(dbTypeT type)
{
  if (db_passwords[type] != "")
    return db_passwords[type].c_str();
  else
    return NULL;
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
    if(!mysql_real_connect(databases[type], db_hosts[type].c_str(), getUser(type), getPass(type), getConnectParam(type), 0, NULL, 0))
    {
      vlogf(LOG_DB, format("Could not connect to database '%s'.") % getConnectParam(type));
      vlogf(LOG_DB, format("%s") % mysql_error(databases[type]));
      return NULL;
    }
  }
  return databases[type];
}


TDatabase::TDatabase(dbTypeT tdb, bool log)
{
  pimpl = new TDatabasePimpl();
  pimpl->db = database_connection.getDB(tdb);
  pimpl->row_count = 0;
  pimpl->log = log;
}

TDatabase::~TDatabase(){
  mysql_free_result(pimpl->res);
  delete pimpl;
}

long TDatabase::lastInsertId()
{
 return pimpl->db ? mysql_insert_id(pimpl->db) : 0;
}

// advance to the next row of the current query
// this is a little funky under postgres.  we initialize row to -1 when
// we do a query, then fetchRow increments it each time.
bool TDatabase::fetchRow(){
  if(!pimpl->res)
    return FALSE;
  
  if(!(pimpl->row=mysql_fetch_row(pimpl->res)))
    return FALSE;
  
  return TRUE;
}

unsigned long TDatabase::escape_string(char *to, const char *from, unsigned long length)
{
  return mysql_real_escape_string(pimpl->db, to, from, length);
}

unsigned long TDatabase::escape_string_ugly(char *to, const char *from, unsigned long length)
{
  TDatabase sn(DB_SNEEZY);
  return sn.escape_string(to, from, length);
}


const sstring TDatabase::operator[] (unsigned int i) const
{
  if(pimpl->res && i > (mysql_num_fields(pimpl->res)-1)){
    vlogf(LOG_DB, format("TDatabase::operator[%i] - invalid column index") % i);
    return empty;
  } else {
    return pimpl->row[i];
  }
}

const sstring TDatabase::operator[] (const sstring &s) const
{
  if(!pimpl->res || !pimpl->row)
    return NULL;

  std::map<const char*, int, ltstr>::const_iterator cur = pimpl->column_names.find(s.lower().c_str());

  if(cur == pimpl->column_names.end()){
    vlogf(LOG_DB, format("TDatabase::operator[%s] - invalid column name") %  s);
    return empty;
  }

  return pimpl->row[(*cur).second];
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
  if (!pimpl)
    return FALSE;
  if(!pimpl->db)
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
	    return FALSE;
	  }

	  fromlen=strlen(from);
	  
	  // mysql_escape_string needs a buffer that is 
	  // (string * 2) + 1 in size to avoid overruns
	  if(((fromlen*2)+1) > tolen){
	    vlogf(LOG_DB, format("query - buffer overrun on %s") % from);
	    vlogf(LOG_DB, format("%s") % qsave);
	    return FALSE;
	  }
	  
	  escape_string(to, from, strlen(from));
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

  if (pimpl->log || (toggleInfo.isLoaded() && toggleInfo[TOG_DBLOGGING]->toggle))
    vlogf(LOG_DB, buf);

  if(mysql_query(pimpl->db, buf.c_str())){
    vlogf(LOG_DB, format("query failed: %s") % mysql_error(pimpl->db));
    vlogf(LOG_DB, format("%s") % buf);
    return FALSE;
  }
  restmp=mysql_store_result(pimpl->db);

  // if there is supposed to be some results from this query,
  // free the previous results (if any) and assign the new results
  // if there aren't supposed to be results (update, insert, delete, etc)
  // then don't do anything with the results, they might still be used
  if(restmp){
    if(pimpl->res)
      mysql_free_result(pimpl->res);
    pimpl->res=restmp;
  }
  
  // capture rowcount here, because the db pointer state changes when
  // db timing is on
  pimpl->row_count = (long) mysql_affected_rows(pimpl->db);

  // store the column names and offsets
  if(pimpl->res){
    pimpl->column_names.clear();
    unsigned int num_fields=mysql_num_fields(pimpl->res);
    MYSQL_FIELD *fields=mysql_fetch_fields(pimpl->res);
    
    for(unsigned int i=0;i<num_fields;++i)
      pimpl->column_names[fields[i].name]=i;
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

    mysql_query(pimpl->db, buf.c_str());
  }


  //  if(res)
  //    vlogf(LOG_DB, "New query results stored.");
  
  return TRUE;
}

bool TDatabase::isResults(){
  if(pimpl->res && mysql_num_rows(pimpl->res))
    return TRUE;

  return FALSE;
}

long TDatabase::rowCount(){
  // return # of affected or retrieved rows
  // -1 if query returned an error
  
  // this gets set in TDatabase::query
  // because the db pointer will have changed state if query timing is on
  return pimpl->row_count;
}
