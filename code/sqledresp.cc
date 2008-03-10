#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"
#include <unistd.h>

int main(int argc, char **argv)
{
  TDatabase db(DB_SNEEZYBETA);
  vector<int>vnums;
  map<sstring,sstring>val;
  char file[]="/tmp/sqledrespXXXXXX";
  FILE *tmpfile;
  sstring buf, sbuf;
  int answer;

  toggleInfo.loadToggles();
  if(argc<=1){
    printf("Usage: sqledresp <vnum list>\n");
    printf("Example: sqledresp 13700-13780 13791 13798\n");
    exit(0);
  }

  if(!parse_num_args(argc-1, argv+1, vnums))
    exit(0);

  for(unsigned int i=0;i<vnums.size();++i){
    printf("Fetching data for response %i\n", vnums[i]);

    db.query("select vnum, response from mobresponses where vnum=%i", vnums[i]);
    if(db.fetchRow()){
      buf = fmt("- response\n"); sbuf+=buf;
      buf = fmt("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = fmt("response~:\n%s~\n") % db["response"];  sbuf+=buf;
      buf = fmt("\n"); sbuf+=buf;
    }
  }


  mkstemp(file);
  tmpfile=fopen(file, "w");
  fprintf(tmpfile, "%s", sbuf.c_str());
  fclose(tmpfile);

  printf("Opening editor.\n");
  buf = fmt("$EDITOR %s") % file;
  system(buf.c_str());


  printf("Insert response%s? [y/n] ", vnums.size()>1?"s":"");
  while(1){
    answer=getc(stdin);
    if(answer=='n'){
      printf("Response%s not inserted.\n", vnums.size()>1?"s":"");
      unlink(file);
      exit(0);
    } else if(answer=='y'){
      break;
    } else {
      printf("Insert response%s? [y/n] ", vnums.size()>1?"s":"");
    }
  }

  buf = fmt("/bin/cp -f %s %s.backup") % file % file;
  system(buf.c_str());


  int i=0;

  while(++i){
    val=parse_data_file(file, i);
    if(val["vnum"]=="EOM")
      break;
    
    if(val["DATATYPE"]=="response"){
      printf("replacing response %s\n", val["vnum"].c_str());
      db.query("delete from mobresponses where vnum=%s",
	       val["vnum"].c_str());
      
      db.query("insert into mobresponses (vnum, response) values (%s,'%s')",
	       val["vnum"].c_str(),val["response"].c_str());
    }

    printf("\n");

  }


  unlink(file);

  printf("Done.\n\r");
  printf("Your backup file is %s.backup\n", file);
}
