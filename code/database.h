#ifndef __DATABASE_H
#define __DATABASE_H

// maintain instances of sneezydb and immodb
class TDatabaseConnection
{
  MYSQL *sneezydb, *immodb;

 public:
  MYSQL *getSneezyDB(){
    if(!sneezydb){
      const char * dbconnectstr = NULL;
      
      if(gamePort == PROD_GAMEPORT){
	dbconnectstr="sneezy";
      } else if(gamePort == BUILDER_GAMEPORT){
	dbconnectstr="sneezybuilder";
      } else {
	dbconnectstr="sneezybeta";
      }
      
      
      vlogf(LOG_DB, "Initializing database '%s'.",
	    dbconnectstr);
      sneezydb=mysql_init(NULL);
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!mysql_real_connect(sneezydb, NULL, "sneezy", NULL, 
			     dbconnectstr, 0, NULL, 0)){
	vlogf(LOG_DB, "Could not connect to database '%s'.",
	      dbconnectstr);
	return NULL;
      }
    }
      
    return sneezydb;
  }    


  MYSQL *getImmoDB(){
    if(!immodb){
      vlogf(LOG_DB, "Initializing database 'immortal'.");
      immodb=mysql_init(NULL);
      
      vlogf(LOG_DB, "Connecting to database.");
      if(!mysql_real_connect(immodb, NULL, "sneezy", NULL, 
			     "immortal", 0, NULL, 0)){
	vlogf(LOG_DB, "Could not connect to database 'immortal'.");
	return NULL;
      }
    }
    
    return immodb;
  }
};



class TDatabase
{
  MYSQL_RES *res;
  MYSQL_ROW row;
  MYSQL *db;
  
 public:
  void setDB(string);
  bool fetchRow();
  char *getColumn(unsigned int);
  bool query(const char *,...);

  TDatabase();
  TDatabase(string);
  ~TDatabase();
};

#endif
