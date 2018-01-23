//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __MAIL_H
#define __MAIL_H

const int MAX_MAIL_SIZE = 4000;

extern bool has_mail(const sstring);
extern void autoMail(TBeing *, const char *, const char *, int m = 0, int r = 0);

#endif
