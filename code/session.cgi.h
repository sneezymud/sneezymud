#ifndef __SESSION_CGI_H
#define __SESSION_CGI_H

#include <cgicc/Cgicc.h>

// TODO:
// add date/time info to session storage in db, so we can expire them
// check for duplicates before trying to use a session id
// add option for persistent logins rather than current-session


// TSession is a class for handling session authentication in cgi
// scripts.  The idea is that the user can login using their sneezy
// account name and password, and we give them a cookie that we can
// check later for authentication.

// Usage:
//
// Cgicc cgi;
// TSession session(cgi, "mudmail");
//
// if(!session.isValid()){
//   // send them to a login form, and when you get a name and password:
//   if(session.checkPasswd(name, passwd)){
//     session.createSession();
//     cout<< HTTPRedirectHeader("mudmail.cgi").setCookie(session.getCookie());
//   } else {
//     // bad login
//   }
// } else {
//   // they are logged in
// }


class TSession {
  // contents of the cookie we send
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
  void createSession();

  // returns false if there is no session id or account id set in the class
  bool isValid();
  // deletes the session id string from the database
  void logout();

  // validates name and passwd, user input.
  bool checkPasswd(sstring name, sstring passwd);

  cgicc::HTTPCookie getCookie();

  int getAccountID(){ return account_id; }
  sstring getSessionID(){ return session_id; }

  TSession(cgicc::Cgicc, sstring);
};

#endif
