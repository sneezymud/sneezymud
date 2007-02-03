#ifndef __FORMAT_H
#define __FORMAT_H

class fmt : public sstring {
  unsigned int last;

 public:
  fmt() : sstring(){ last=0; }
  fmt(const char *str) : sstring(str) { last=0; }
  fmt(const string &str) : sstring(str) { last=0; }

  template <class T> sstring doFormat(const sstring &, const T &);
  sstring doFormat(const sstring &, const sstring &);
  sstring doFormat(const sstring &, const string &);

  template <class T> fmt & operator%(const T &s);
};


template <class T> sstring fmt::doFormat(const sstring &fmt, const T &x)
{
  unsigned int MY_MAX_STRING_LENGTH=MAX_STRING_LENGTH * 2;
  char buf[MY_MAX_STRING_LENGTH];

  snprintf(buf, MY_MAX_STRING_LENGTH, fmt.c_str(), x);

  if(strlen(buf) == MY_MAX_STRING_LENGTH - 1){
    vlogf(LOG_BUG, "fmt::doFormat(): buffer reached MAX_STRING_LENGTH");

    // can't use fmt here of course
    vlogf(LOG_BUG, sstring("fmt::doFormat(): buffer=")+
	  sstring(buf).substr(70));
  }

  return (sstring) buf;
}


template <class T> fmt & fmt::operator %(const T &x)
{
  sstring buf, output;
  bool found=false;
  
  output=substr(0, last);

  for(unsigned int i=last;i<size();++i){
    if((*this)[i]=='%'){
      // skip %%
      if((i+1)<size() && (*this)[i+1]=='%'){
	++i;
	output += "%";
	continue;
      }

      // first grab the format specifier
      for(buf="%",++i;i<size() && (*this)[i]!='%';++i){
	buf += (*this)[i];
      }

      // now do the print
      output += doFormat(buf, x);
      last = output.size();

      // we're just doing one format specifier for this arg, so copy
      // the rest of the source string and exit
      for(;i<size();++i)
	output += (*this)[i];

      found=true;

      break;
    }
    
    output += (*this)[i];
  }

  if(!found)
    vlogf(LOG_BUG,fmt("format passed argument with no format specifier, output=%s") % 
	  output.c_str());

  this->assign(output);

  return (*this);
}


#endif
