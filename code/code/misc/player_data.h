#pragma once

#include "database.h"
#include <memory>

class sstring;
class charFile;

bool load_char(const sstring &name, charFile *char_element, std::unique_ptr<IDatabase> dbase = nullptr);
