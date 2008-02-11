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
  const sstring replaceString(sstring, sstring) const;
  const sstring trim() const;
  const sstring capitalizeSentences() const;
  const sstring matchCase(const sstring match) const;

  // call this function with NULL data to get alloc size, then agian with alloc'd array
  int split(const char delimit, sstring *data) const;

  // other functions
  const bool hasDigit() const;
  const bool isNumber() const;
  const bool isWord() const;
  const bool startsVowel() const;

  // inlines
  void inlineReplaceString(const string f, const string r) {
    string::size_type start = 0;
    while(string::npos != (start = find(f, start)))
    {
      replace(start, f.length(), r.c_str(), r.length());
      start += r.length();
    }
  }

};

// used for easily defining buffer sizes
#ifndef cElements
#define cElements(x) (sizeof(x)/sizeof(x[0]))
#endif


#endif
