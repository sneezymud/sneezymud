#ifndef __SSTRING_H
#define __SSTRING_H

class fmt;


class sstring : public string {
public:
  // constructors
  sstring() : string(){}
  sstring(const char *str) : string(str?str:"") {}
  sstring(const string &str) : string(str) {}

  const sstring & operator=(fmt &a);

  // these functions return modified copies of the string
  const sstring toCRLF() const;
  const sstring lower() const;
  const sstring upper() const;
  const sstring cap() const;
  const sstring uncap() const;
  const sstring word(int) const;
  const sstring comify() const;

  // other functions
  const bool hasDigit() const;
  const bool isNumber() const;
  const bool isWord() const;
  const bool startsVowel() const;

};


#endif
