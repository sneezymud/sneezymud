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

  template <class T> fmt & operator%(const T &s);
};


template <class T> sstring fmt::doFormat(const sstring &fmt, const T &x)
{
  sstring buf;

  ssprintf(buf, fmt.c_str(), x);

  return buf;
}



template <class T> fmt & fmt::operator %(const T &x)
{
  sstring buf, output;
  bool found=false;
  
  output=substr(0, last);

  for(unsigned int i=last;i<size();++i){
    if((*this)[i]=='%'){
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
    vlogf(LOG_BUG,"format passed argument with no format specifier, output=%s",
	  output.c_str());

  this->assign(output);

  return (*this);
}


#endif
