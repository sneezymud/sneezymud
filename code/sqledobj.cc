#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"
#include<unistd.h>

// This should be a perl script
// but I feel like doing it in c++
// whee system()
// this code pretty much sucks

const char *pgcmd="pg_dump --no-quotes --no-reconnect --data-only --column-inserts --table=%s %s | grep -E \"%s\" | grep \"%s\" | perl -pe \"s/\',/\',\n/g; s/\\134012/\n/g;s/VALUES /VALUES\n/;s/INSERT/\nINSERT/g;\" >> %s";

int main(int argc, char **argv)
{
  string immortal, db, buf, where, grep;
  char file[]="/tmp/sqledobjXXXXXX";
  int answer;
  unsigned int i;
  FILE *tmpfile;
  vector<int>vnums;

  if(argc<=1 || argc>3){
    printf("Usage: sqledobj <owner> <vnum list>\n");
    printf("if <owner> is given (optional), then the immortal database is used\n");
    printf("Example: sqledobjs.pl Peel 13700-13780 13791 13798\n");
    exit(0);
  }

  if(!isdigit(argv[1][0])){
    db="immortal";
    immortal=argv[1];
    if(!parse_num_args(argc-2, argv+2, vnums))
      exit(0);
   
    ssprintf(where, "where vnum in (");
    for (i=0;i<vnums.size()-1;i++){
      ssprintf(where, "%s%i,", where.c_str(),vnums[i]);
      ssprintf(grep, "%sVALUES \\(%i,|",grep.c_str(), vnums[i]);
    }
    ssprintf(where, "%s%i) and owner='%s'", where.c_str(), vnums[i], immortal.c_str());
    ssprintf(grep, "%sVALUES \\(%i,",grep.c_str(), vnums[i]);
  } else {
    db="sneezybeta";
    immortal="";
    if(!parse_num_args(argc-1, argv+1, vnums))
      exit(0);
    ssprintf(where, "where vnum in (");
    for (i=0;i<vnums.size()-1;i++){
      ssprintf(where, "%s%i,", where.c_str(),vnums[i]);
      ssprintf(grep, "%sVALUES \\(%i,|",grep.c_str(), vnums[i]);
    }
    ssprintf(where, "%s%i)", where.c_str(), vnums[i]);
    ssprintf(grep, "%sVALUES \\(%i,",grep.c_str(), vnums[i]);
  }

  printf("Using database %s.\n", db.c_str());

  mkstemp(file);

  tmpfile=fopen(file, "w");
  fprintf(tmpfile, "delete from obj %s;\n", where.c_str());
  fprintf(tmpfile, "delete from objaffect %s;\n", where.c_str());
  fprintf(tmpfile, "delete from objextra %s;\n", where.c_str());
  fclose(tmpfile);

  printf("Fetching obj data.\n");
  ssprintf(buf, pgcmd, "obj", db.c_str(), grep.c_str(), immortal.c_str(), file);
  system(buf.c_str());

  printf("Fetching objaffect data.\n");
  ssprintf(buf, pgcmd, "objaffect", db.c_str(), grep.c_str(), immortal.c_str(), file);
  system(buf.c_str());

  printf("Fetching objextra data.\n");
  ssprintf(buf, pgcmd, "objextra", db.c_str(), grep.c_str(), immortal.c_str(), file);
  system(buf.c_str());

  ssprintf(buf, "/bin/cp -f %s %s.`whoami`", file, file);
  system(buf.c_str());
  ssprintf(buf, "/bin/chmod a+rw %s.`whoami`", file);
  system(buf.c_str());
  
  printf("Opening editor.\n");
  ssprintf(buf, "$EDITOR %s", file);
  system(buf.c_str());

  printf("Insert this object? [y/n] ");
  while(1){
    answer=getc(stdin);
    if(answer=='n'){
      printf("Object not inserted.\n");
      unlink(file);
      exit(0);
    } else if(answer=='y'){
      break;
    } else {
      printf("Insert this object? [y/n] ");
    }
  }

  ssprintf(buf, "/usr/bin/psql %s < %s", db.c_str(), file);
  system(buf.c_str());

  unlink(file);

  printf("Done. Your backup file is: ");
  fflush(stdout);
  ssprintf(buf, "/bin/ls %s*", file);
  system(buf.c_str());
  printf("\n");
}


