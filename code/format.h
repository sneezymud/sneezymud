#ifndef __FORMAT_H
#define __FORMAT_H

class fmt : public sstring {
 public:
  fmt() : sstring(){}
  fmt(const char *str) : sstring(str) {}
  fmt(const string &str) : sstring(str) {}

  sstring doFormat(const sstring &, const int &);
  sstring doFormat(const sstring &, const double &);
  sstring doFormat(const sstring &, const char &);
  sstring doFormat(const sstring &, const sstring &);

  template <class T> fmt & operator%(const T &s);
};


sstring doFormat(const sstring &fmt, const int &x)
{
  sstring buf;

  if(fmt.find("diouXx")==sstring::npos){
    ssprintf(buf, "bad format specifier (%s) used for int", fmt.c_str());
    vlogf(LOG_BUG, buf.c_str());
  } else {
    ssprintf(buf, fmt.c_str(), x);
  }

  return buf;
}


sstring doFormat(const sstring &fmt, const double &x)
{
  sstring buf;

  if(fmt.find("feEgG")==sstring::npos){
    ssprintf(buf, "bad format specifier (%s) used for double", fmt.c_str());
    vlogf(LOG_BUG, buf.c_str());
  } else {
    ssprintf(buf, fmt.c_str(), x);
  }

  return buf;
}

sstring doFormat(const sstring &fmt, const char &x)
{
  sstring buf;

  if(fmt.find("c")==sstring::npos){
    ssprintf(buf, "bad format specifier (%s) used for char", fmt.c_str());
    vlogf(LOG_BUG, buf.c_str());
  } else {
    ssprintf(buf, fmt.c_str(), x);
  }

  return buf;
}

sstring doFormat(const sstring &fmt, const sstring &x)
{
  sstring buf;

  if(fmt.find("s")==sstring::npos){
    ssprintf(buf, "bad format specifier (%s) used for sstring", fmt.c_str());
    vlogf(LOG_BUG, buf.c_str());
  } else {
    ssprintf(buf, fmt.c_str(), x.c_str());
  }

  return buf;
}


template <class T> fmt & fmt::operator %(const T &x)
{
  sstring buf, sbuf, output;

  for(unsigned int i=0;i<size();++i){
    if((*this)[i]=='%'){
      // first grab the format specifier
      for(buf="";i<size() && !isalpha((*this)[i]);++i){
	buf += (*this)[i];
      }
      
      if(i<size())
	buf += (*this)[i];

      // now do the print
      output += doFormat(buf, x);

      // we're just doing one format specifier for this arg, so copy
      // the rest of the source string and exit
      for(++i;i<size();++i)
	output += (*this)[i];
      break;
    }
    
    output += (*this)[i];
  }

  this->assign(output);

  return (*this);
}


#endif
