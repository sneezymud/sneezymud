#include "stdsneezy.h"
#include "database.h"

// parses args like "13700-13780 13791 13798"
bool parse_num_args(int argc, char **argv, vector<int> &vnums)
{
  int s, e, n;
  sstring tmp;
  unsigned int npos;

#if 0
  printf("argc=%i\n", argc);
  for(int i=0;i<argc;++i){
    printf("argv[%i]=%s\n", i, argv[i]);
  }
#endif

  for(int i=0;i<=(argc-1);i++){
    tmp=argv[i];
    npos=tmp.find("-");

    if(npos != sstring::npos){
      s=convertTo<int>(tmp.substr(0, npos));
      e=convertTo<int>(tmp.substr(npos+1, tmp.size()));
      
      if(s==0 || e==0){
	printf("Bad argument %s, aborting.\n", tmp.c_str());
	return false;
      }

      if(s>e){
	int tmp;
	tmp=s;
	s=e;
	e=tmp;

	//	s^=e^=s^=e; // swap
      }

      while(s<=e){
	vnums.push_back(s++);
      }
    } else {
      n=convertTo<int>(tmp);

      if(n==0){
	printf("Bad argument %s, aborting.\n", tmp.c_str());
	return false;
      }

      vnums.push_back(n);
    }
  }
  return true;
}


map <sstring,sstring> parse_data_file(const sstring &file, int num)
{
  ifstream ifile(file.c_str());
  sstring buf, name, val, type;
  map <sstring,sstring> values;
  unsigned int loc;

  // this is a crappy kluge
  values["vnum"]="EOM";
  values["shop_nr"]="EOM";
      
  while(num--){
    while(getline(ifile, buf)){
      if(buf[0]=='-'){
	values["DATATYPE"]=buf.substr(2);
	break;
      }
    }
  }


  while(getline(ifile, buf)){
    if(buf[0]=='-')
      break;

    if(!isalpha(buf[0]))
      continue;

    if((loc=buf.find_first_of("~"))!=sstring::npos){
      name=buf.substr(0, loc);
      val="";
      
      while(getline(ifile, buf) && buf.find_first_of("~")==sstring::npos){
	val+=buf;
	val+="\n";
      }
      // this handles the stuff on the line with the ~
      if(buf[0]!='~' && buf[0]!='\n'){
	// strip ~
	buf.erase(buf.size()-1, 1);
	val+=buf;
      }
    } else {
      if((loc=buf.find_first_of(":"))==sstring::npos)
	continue;
      
      name=buf.substr(0, loc);
      val=buf.substr(loc+1);
    }

    if(val.find_first_not_of(" ")==sstring::npos){
      val="";
    } else {
      val=val.substr(val.find_first_not_of(" "));
    }

    values[name]=val;
  }

  return values;
}
