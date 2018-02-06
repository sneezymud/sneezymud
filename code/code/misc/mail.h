#pragma once

const int MAX_MAIL_SIZE = 4000;

extern bool has_mail(const sstring &);
extern void autoMail(TBeing *, const char *, const char *, int m = 0, int r = 0);
