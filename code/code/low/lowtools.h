#ifndef __LOWTOOLS_H
#define __LOWTOOLS_H

class sstring;

bool parse_num_args(int, char **, std::vector<int> &);
std::map <sstring,sstring> parse_data_file(const sstring &file, int num);

#endif
