#ifndef __DATABASE_H
#define __DATABASE_H

#include <libpq-fe.h>

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

enum dbTypeT {
  DB_SNEEZY,
  DB_SNEEZYBETA,
  DB_IMMORTAL,
  DB_SNEEZYGLOBAL,
  DB_SNEEZYBUILDER,
  DB_SNEEZYPROD,
};

// we return this instead of null if they try to fetch an invalid column
const sstring empty="";


class TDatabase
{
  PGresult *res;
  int row;
  PGconn *db;
  
 public:
  void setDB(dbTypeT);
  bool query(const char *,...);
  bool fetchRow();
  const sstring operator[] (const sstring &) const;
  bool isResults();


  TDatabase();
  TDatabase(dbTypeT);
  ~TDatabase();
};

// maintain instances of sneezydb and immodb
class TDatabaseConnection
{
  PGconn *sneezydb, *immodb, *sneezyglobaldb, *sneezybetadb, *sneezybuilderdb;
  PGconn *sneezyproddb;

 public:
  PGconn *getSneezyDB(){
    if(!sneezydb){
      const char * dbconnectstr = NULL;
      
      if(gamePort == PROD_GAMEPORT){
	dbconnectstr="dbname=sneezy";
      } else if(gamePort == BUILDER_GAMEPORT){
	dbconnectstr="dbname=sneezybuilder";
      } else {
	dbconnectstr="dbname=sneezybeta";
      }
            
      vlogf(LOG_DB, fmt("Initializing database '%s'.") % 
	    dbconnectstr);
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!(sneezydb=PQconnectdb(dbconnectstr))){
	vlogf(LOG_DB, fmt("Could not connect to database '%s'.") % 
	      dbconnectstr);
	return NULL;
      }
    }
      
    return sneezydb;
  }

  PGconn *getSneezyProdDB(){
    if(!sneezyproddb){
      vlogf(LOG_DB, "Initializing database 'sneezy'.");
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!(sneezyproddb=PQconnectdb("dbname=sneezy"))){
	vlogf(LOG_DB, "Could not connect to database 'sneezy'.");
	return NULL;
      }
    }
    
    return sneezyproddb;
  }


  PGconn *getSneezyBuilderDB(){
    if(!sneezybuilderdb){
      vlogf(LOG_DB, "Initializing database 'sneezybuilder'.");
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!(sneezybuilderdb=PQconnectdb("dbname=sneezybuilder"))){
	vlogf(LOG_DB, "Could not connect to database 'sneezybuilder'.");
	return NULL;
      }
    }
    
    return sneezybuilderdb;
  }

  PGconn *getSneezyBetaDB(){
    if(!sneezybetadb){
      vlogf(LOG_DB, "Initializing database 'sneezybeta'.");
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!(sneezybetadb=PQconnectdb("dbname=sneezybeta"))){
	vlogf(LOG_DB, "Could not connect to database 'sneezybeta'.");
	return NULL;
      }
    }
    
    return sneezybetadb;
  }


  PGconn *getImmoDB(){
    if(!immodb){
      vlogf(LOG_DB, "Initializing database 'immortal'.");
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!(immodb=PQconnectdb("dbname=immortal"))){
	vlogf(LOG_DB, "Could not connect to database 'immortal'.");
	return NULL;
      }
    }
    
    return immodb;
  }

  PGconn *getSneezyGlobalDB(){
    if(!sneezyglobaldb){
      vlogf(LOG_DB, "Initializing database 'sneezyglobal'.");
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!(sneezyglobaldb=PQconnectdb("dbname=sneezyglobal"))){
	vlogf(LOG_DB, "Could not connect to database 'sneezyglobal'.");
	return NULL;
      }
    }
    
    return sneezyglobaldb;
  }
  
};



#endif


