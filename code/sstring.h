#ifndef __SSTRING_H
#define __SSTRING_H

class sstring : public string {
public:
  // constructors
  sstring() : string(){}
  sstring(const char *str) : string(str?str:"") {}
  sstring(const string &str) : string(str) {}

  // extension functions
  const sstring toCRLF() const;
  const sstring lower() const;
  const sstring upper() const;
  const sstring cap() const;
  const sstring uncap() const;
  const sstring word(int) const;
};


#endif
