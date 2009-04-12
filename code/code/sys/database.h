#ifndef __DATABASE_H
#define __DATABASE_H

#include <mysql/mysql.h>
#include <queue>
#include <map>

class sstring;

// TDatabase is a class for interacting with the sql database.
//
// You should always use local instances of TDatabase, do not use a pointer
// and the new operator.  The local instance will clean up after itself in
// its destructor when it goes out of scope.  All of the functions are safe
// to use even on failures, so if you do not do any error checking, the worst
// than can happen is that you won't get any results.  On error, TDatabase
// will send LOG_DB vlogf's and return the appropriate value (false or NULL).
//
// Usage example:
//
// #include "database.h"
//
// float weight=5.5;
// char name[]="blade";
// int vnum=10000;
//
// TDatabase db("sneezy");
// db.query("select vnum, price, short_desc from obj where weight<%f and
// name like '%%%s%%' and vnum>%i", weight, name, vnum);
//
// while(db.fetchRow()){
//   if(atoi(db["price"]) > 10000){
//     vlogf(LOG_BUG, fmt("item %s had value of %s") %  db["vnum"] % db["price"]);
//   }
//   sendTo("%s %s", db["vnum"], db["short_desc"]);
// }
//
//
// Documentation:
//
// TDatabase(dbTypeT) - The initializer takes the name of the database you 
// want to use as an argument.  Allowable databases are listed below under
// dbTypeT, but the most common is DB_SNEEZY.
// Returns: TDatabase (initializer)
// Ex: TDatabase db(DB_SNEEZY);
//
// bool setDB(dbTypeT) - This function sets the database that the instance 
// will use, and is generally called from the constructor rather than directly.
// Returns: nothing (void)
// Ex: db.setDB(DB_SNEEZY);
//
// bool query(const char*,...) - This function sends a query to the database.
// It takes a printf style format sstring as the arguments.  The allowed
// specifiers are %s (char *), %i (int), %f (double) and %% (to print a %).
// The arguments that are passed are escaped for the query.  If the query
// does not expect results (insert, update, delete, etc) then the results are
// left as is.  You can do a select, then do an insert/update/delete and still
// access the select's results.
// Returns: TRUE if query was sent successfully, FALSE if there was an error
// Ex: 
// float weight=5.5;
// char name[]="blade";
// int vnum=10000;
// db.query("select vnum, short_desc from obj where weight<%f and
// name like '%%%s%%' and vnum>%i", weight, name, vnum);
//
// bool fetchRow() - Makes the next row of results available via getColumn.
// Returns: FALSE if no results or no more rows available.
// Ex:
// while(db.fetchRow(){
//   printf("%s", db["vnum"]);
// }
//
// char *operator[](const sstring &) - returns the data associated with
// the specified column (by name)
// Ex:
// db.query("select vnum, short_desc from obj");
// db.fetchRow();
// sstring short_desc = db["short_desc"];
//
// bool isResults() - checks if there are results available
// Returns: TRUE if results are there, FALSE if not
// 
// long rowCount() - added to return affected or retrieved row counts
// This should include affected counts for inserts, updates and deletes
// as well as standard result set sizes for select statements.
// Result of -1 means the query returned an error.
// Although, the docs claim that the my_ulonglong datatype is unsigned so who knows?

enum dbTypeT {
  DB_SNEEZY,
  DB_SNEEZYBETA,
  DB_IMMORTAL,
  DB_SNEEZYGLOBAL,
  DB_SNEEZYPROD,
  DB_SNEEZYBUILDER,
  DB_WIKI_MORTAL,
  DB_WIKI_BUILDER,
  DB_WIKI_ADMIN,
  DB_FORUMS_ADMIN,

  DB_MAX,
};

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

class TDatabase
{
  MYSQL_RES *res;
  MYSQL_ROW row;
  MYSQL *db;
  long row_count;
  std::map <const char *, int, ltstr> column_names;
  
 public:
  void setDB(dbTypeT);
  bool query(const char *,...);
  bool fetchRow();
  const sstring operator[] (const sstring &) const;
  const sstring operator[] (unsigned int) const;
  bool isResults();
  long rowCount();
  long lastInsertId() { return db ? mysql_insert_id(db) : 0; }

  TDatabase();
  TDatabase(dbTypeT);
  ~TDatabase();
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

  // shortcuts - not sure if they are really needed...
  MYSQL *getSneezyDB() { return getDB(DB_SNEEZY); }
  MYSQL *getSneezyProdDB() { return getDB(DB_SNEEZYPROD); }
  MYSQL *getSneezyBetaDB() { return getDB(DB_SNEEZYBETA); }
  MYSQL *getImmoDB() { return getDB(DB_IMMORTAL); }
  MYSQL *getSneezyGlobalDB() { return getDB(DB_SNEEZYGLOBAL); }

};

extern std::vector <std::string> db_hosts;
extern std::queue<sstring> queryqueue;
extern TDatabaseConnection database_connection;

#endif


