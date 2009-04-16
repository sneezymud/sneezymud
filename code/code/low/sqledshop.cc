#include "toggle.h"
#include "sstring.h"
#include "configuration.h"
#include "database.h"
#include "lowtools.h"
#include <unistd.h>

int main(int argc, char **argv)
{
  Config::doConfiguration();
  TDatabase db(DB_SNEEZYBETA);
  std::vector<int>vnums;
  std::map<sstring,sstring>val;
  char file[]="/tmp/sqledshopXXXXXX";
  FILE *tmpfile;
  sstring buf, sbuf;
  int answer;

  if(argc<=1){
    printf("Usage: sqledshop <shop_nr list>\n");
    printf("Example: sqledshop 100\n");
    exit(0);
  }

  toggleInfo.loadToggles();

  if(!parse_num_args(argc-1, argv+1, vnums))
    exit(0);

  for(unsigned int i=0;i<vnums.size();++i){
    printf("Fetching data for shop %i\n", vnums[i]);

    db.query("select * from shop where shop_nr=%i", vnums[i]);
    if(db.fetchRow()){
      buf = format("- shop\n"); sbuf+=buf;
      buf = format("shop_nr:       %s\n") % db["shop_nr"];  sbuf+=buf;
      buf = format("profit_buy:    %s\n") % db["profit_buy"];  sbuf+=buf;
      buf = format("profit_sell:   %s\n") % db["profit_sell"];  sbuf+=buf;
      buf = format("no_such_item1: %s\n") % db["no_such_item1"];  sbuf+=buf;
      buf = format("no_such_item2: %s\n") % db["no_such_item2"];  sbuf+=buf;
      buf = format("do_not_buy:    %s\n") % db["do_not_buy"];  sbuf+=buf;
      buf = format("missing_cash1: %s\n") % db["missing_cash1"];  sbuf+=buf;
      buf = format("missing_cash2: %s\n") % db["missing_cash2"];  sbuf+=buf;
      buf = format("message_buy:   %s\n") % db["message_buy"];  sbuf+=buf;
      buf = format("message_sell:  %s\n") % db["message_sell"];  sbuf+=buf;
      buf = format("temper1:       %s\n") % db["temper1"];  sbuf+=buf;
      buf = format("temper2:       %s\n") % db["temper2"];  sbuf+=buf;
      buf = format("keeper:        %s\n") % db["keeper"];  sbuf+=buf;
      buf = format("flags:         %s\n") % db["flags"];  sbuf+=buf;
      buf = format("in_room:       %s\n") % db["in_room"];  sbuf+=buf;
      buf = format("open1:         %s\n") % db["open1"];  sbuf+=buf;
      buf = format("close1:        %s\n") % db["close1"];  sbuf+=buf;
      buf = format("open2:         %s\n") % db["open2"];  sbuf+=buf;
      buf = format("close2:        %s\n") % db["close2"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from shoptype where shop_nr=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- shoptype\n"); sbuf+=buf;
      buf = format("shop_nr: %s\n") % db["shop_nr"];  sbuf+=buf;
      buf = format("type:    %s\n") % db["type"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }

    db.query("select * from shopproducing where shop_nr=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- shopproducing\n"); sbuf+=buf;
      buf = format("shop_nr: %s\n") % db["shop_nr"];  sbuf+=buf;
      buf = format("producing:    %s\n") % db["producing"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from shopmaterial where shop_nr=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- shopmaterial\n"); sbuf+=buf;
      buf = format("shop_nr: %s\n") % db["shop_nr"];  sbuf+=buf;
      buf = format("mat_type:    %s\n") % db["mat_type"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }

  }


  mkstemp(file);
  tmpfile=fopen(file, "w");
  fprintf(tmpfile, "%s", sbuf.c_str());
  fclose(tmpfile);

  printf("Opening editor.\n");
  buf = format("$EDITOR %s") % file;
  system(buf.c_str());


  printf("Insert shop%s? [y/n] ", vnums.size()>1?"s":"");
  while(1){
    answer=getc(stdin);
    if(answer=='n'){
      printf("Shop%s not inserted.\n", vnums.size()>1?"s":"");
      unlink(file);
      exit(0);
    } else if(answer=='y'){
      break;
    } else {
      printf("Insert shop%s? [y/n] ", vnums.size()>1?"s":"");
    }
  }

  buf = format("/bin/cp -f %s %s.backup") % file % file;
  system(buf.c_str());


  int i=0;

  while(++i){
    val=parse_data_file(file, i);
    if(val["shop_nr"]=="EOM")
      break;
    
    if(val["DATATYPE"]=="shop"){
      printf("replacing shop %s\n", val["shop_nr"].c_str());
      db.query("delete from shop where shop_nr=%s",
	       val["shop_nr"].c_str());
      db.query("delete from shopmaterial where shop_nr=%s",
	       val["shop_nr"].c_str());
      db.query("delete from shoptype where shop_nr=%s",
	       val["shop_nr"].c_str());
      db.query("delete from shopproducing where shop_nr=%s",
	       val["shop_nr"].c_str());

  
      
      db.query("insert into shop (shop_nr,profit_buy,profit_sell,no_such_item1,no_such_item2,do_not_buy,missing_cash1,missing_cash2,message_buy,message_sell,temper1,temper2,keeper,flags,in_room,open1,close1,open2,close2) values (%s,%s,%s,'%s','%s','%s','%s','%s','%s','%s',%s,%s,%s,%s,%s,%s,%s,%s,%s)",
	       val["shop_nr"].c_str(),val["profit_buy"].c_str(),val["profit_sell"].c_str(),val["no_such_item1"].c_str(),val["no_such_item2"].c_str(),val["do_not_buy"].c_str(),val["missing_cash1"].c_str(),val["missing_cash2"].c_str(),val["message_buy"].c_str(),val["message_sell"].c_str(),val["temper1"].c_str(),val["temper2"].c_str(),val["keeper"].c_str(),val["flags"].c_str(),val["in_room"].c_str(),val["open1"].c_str(),val["close1"].c_str(),val["open2"].c_str(),val["close2"].c_str());
    } else if(val["DATATYPE"]=="shoptype"){
      printf("replacing shoptype %s\n", val["shop_nr"].c_str());
      
      db.query("insert into shoptype (shop_nr, type) values (%s,%s)", val["shop_nr"].c_str(), val["type"].c_str());

    } else if(val["DATATYPE"]=="shopproducing"){
      printf("replacing shopproducing %s\n", val["shop_nr"].c_str());
      
      db.query("insert into shopproducing (shop_nr, producing) values (%s,%s)", val["shop_nr"].c_str(), val["producing"].c_str());

    } else if(val["DATATYPE"]=="shopmaterial"){
      printf("replacing shopmaterial %s\n", val["shop_nr"].c_str());
      
      db.query("insert into shopmaterial (shop_nr, mat_type) values (%s,%s)", val["shop_nr"].c_str(), val["mat_type"].c_str());
      

    }

    printf("\n");

  }


  unlink(file);

  printf("Done.\n\r");
  printf("Your backup file is %s.backup\n", file);
}
