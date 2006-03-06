#ifndef __SESSION_CGI_H
#define __SESSION_CGI_H

#include <cgicc/Cgicc.h>

class TSession {
  sstring session_id;
  int account_id;

  sstring cookiename;

  // probably a better way to deal with this cgi stuff but I'm too lazy to
  // figure it out.  we use it in this class to get the cookie info.
  // trying to store it as a non-pointer variable causes runtime errors.
  cgicc::Cgicc *cgi;

  sstring getSessionCookie();
  sstring generateSessionID();
  int validateSessionID();

public:
  void createSession(int account_id);

  bool isValid();
  void logout();

  int getAccountID(){ return account_id; }
  sstring getSessionID(){ return session_id; }

  TSession(cgicc::Cgicc, sstring);
};

#endif
