#ifndef __FORMAT_H
#define __FORMAT_H

class format : public sstring {
 public:
  format() : sstring(){}
  format(const char *str) : sstring(str) {}
  format(const string &str) : sstring(str) {}


  template <class T> format & operator%(const T &s);
};

sstring doFormat(int x)
{
  sstring buf;
  ssprintf(buf, "%i", x);
  return buf;
}


sstring doFormat(double x)
{
  sstring buf;
  ssprintf(buf, "%f", x);
  return buf;
}

sstring doFormat(sstring x)
{
  return x;
}


template <class T> format & format::operator %(const T &x)
{
  sstring buf, sbuf;
  bool found=false;

  vlogf(LOG_PEEL, "got here %s", c_str());

  for(unsigned int i=0;i<size();++i){
    if(i==size()-1 && (*this)[i]=='%'){
      vlogf(LOG_BUG, "found %% at end of format in format::operator%%");      
    } else if(!found && (*this)[i]=='%'){
      switch((*this)[++i]){
	case 's':
	  sbuf += x;
	  found=true;
	  break;
	case 'i':
	  sbuf += doFormat(x);
	  found=true;
	  break;
	case 'f':
	  sbuf += doFormat(x);
	  found=true;
	  break;
	case '%':
	  sbuf += '%';
	  break;
	default:
	  vlogf(LOG_BUG, "format %% - bad format specifier - %c", (*this)[i]);
	  vlogf(LOG_BUG, "%s", c_str());
	  sbuf += '%';
	  sbuf += (*this)[i];
	  found=true;
      }
    } else {
      sbuf += (*this)[i];
    }

    vlogf(LOG_PEEL, "sbuf=%s", sbuf.c_str());
  }

  if(!found){
    vlogf(LOG_BUG, "excess argument to format - %s", c_str());
  }

  this->assign(sbuf);

  vlogf(LOG_PEEL, "after =, %s", c_str());

  return (*this);
}





#endif
