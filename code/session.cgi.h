#ifndef __SESSION_CGI_H
#define __SESSION_CGI_H

#include <cgicc/Cgicc.h>

// TSession is a class for handling session authentication in cgi
// scripts.  The idea is that the user can login using their sneezy
// account name and password, and we give them a cookie that we can
// check later for authentication.  The cookie only keeps them logged
// in for the current browser session, but an option for long-term
// logging in could be added.

// the primary security weakness is that session ids are stored in the
// database until the user logs in again to create a new one, even if
// the session cookie has expired.  This code needs to be modified to
// store a date along with the session id, so that we can be sure no one
// will be able to login with a guessed session id that has expired.

// Usage:
//
// 1. initialize a TSession
// 2. check isValid(), if true then user is already logged in, otherwise:
// 3. send them to a login page.  after you've done a password check,
//    call createSession(account_id), then send them a cookie:
//    setCookie(HTTPCookie("mudmail",session.getSessionID()));
// 
// Cgicc cgi;
// TSession session(cgi, "mudmail");
//
// if(!session.isValid()){
//   // send to login form
//   session.createSession(account_id);
//   cout << HTTPRedirectHeader("mudmail.cgi").setCookie(HTTPCookie("mudmail",session.getSessionID().c_str()));
// } else {
//   // user is logged in already
// }


class TSession {
  // contents of the cookie we send - hopefully unguessable string
  sstring session_id;
  // sneezy account id
  int account_id;

  // name of the cookie we store on the user machine, eg "mudmail"
  sstring cookiename;

  // probably a better way to deal with this cgi stuff but I'm too lazy to
  // figure it out.  we use it in this class to get the cookie info.
  // trying to store it as a non-pointer variable causes runtime errors.
  cgicc::Cgicc *cgi;

  // uses the cgicc object to find the cookie and returns the session id
  sstring getSessionCookie();
  // generates a hopefully unguessable session id string.
  sstring generateSessionID();
  // pulls out the account id that is associated with the stored session id
  // returns -1 if no account id is found (ie session id is invalid)
  int validateSessionID();

public:
  // creates a new session id string and inserts/replaces it into the database
  // for the given account id
  void createSession(int account_id);

  // returns false if there is no session id or account id set in the class
  bool isValid();
  // deletes the session id string from the database
  void logout();

  int getAccountID(){ return account_id; }
  sstring getSessionID(){ return session_id; }

  TSession(cgicc::Cgicc, sstring);
};

#endif
