#pragma once

class sstring;

bool parse_num_args(int, char**, std::vector<int>&);
std::map<sstring, sstring> parse_data_file(const sstring& file, int num);
