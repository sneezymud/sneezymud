---
title: CGI Web Interface Security
category: critical
keywords: [TSession, generateSessionID, checkPasswd, hasWizPower, SQL injection, CSRF, session fixation, cookie security, mysql_real_escape_string, cgisession, escape_html]
related: [persistence-storage.md]
primary_symbols:
  functions: [generateSessionID, checkPasswd, createSession, hasWizPower, escape_html]
  classes: [TSession, TDatabase]
  files: [code/cgi/session.cgi.cc, code/cgi/mudmail.cgi.cc, code/cgi/objlog.cgi.cc]
---

## Overview

SneezyMUD exposes builder tools, player utilities, and game data through a browser-based CGI interface. Players access mail, shops, and equipment calculators. Builders edit zones, objects, and rooms. Some data endpoints are publicly accessible without authentication.

This interface has accumulated significant security debt over decades of development. Session tokens use weak entropy sources, forms lack CSRF protection, unauthenticated endpoints expose game data, and SQL injection vectors exist in some endpoints. The system uses legacy CGI technology with minimal security framework support.

Understanding this system matters because vulnerabilities enable session hijacking, unauthorized data access, SQL injection attacks, and cross-site request forgery against authenticated users. The security model relies on patterns that must be followed consistently across all endpoints, but some endpoints deviate from safe practices.

### Authentication Flow

A player visits the login page and submits credentials. The system validates username and password against the database using legacy crypt() hashing. On success, it generates a session ID from weak entropy sources (current time, unseeded random, process ID, stack address), hashes them with MD5, stores the result in the cgisession database table, and returns it as an HTTP cookie. Subsequent requests include this cookie. The server validates sessions by checking that the stored timestamp plus duration exceeds current time and retrieves the associated account ID.

Builder tools add a second layer: after validating the session, they query the wizpower table to check if any character on the account has the required power (POWER_BUILDER, POWER_LOAD, etc.). This account-level authorization means one privileged character grants access to all tools for that account.

Some endpoints skip authentication entirely, exposing corporation listings and quest data to any web visitor.

### Security Architecture Weaknesses

The session ID generation algorithm combines four low-entropy inputs. Current timestamp is predictable to the second. The random() function uses no visible seed, defaulting to 1. Process ID is observable through /proc or predictable from sequential allocation. Stack addresses have limited entropy, especially with ASLR disabled. Hashing these with MD5 doesn't add entropy, only obscures the predictable inputs.

Password verification truncates crypt() output to 10 characters, reducing cryptographic strength. The crypt() function itself defaults to DES hashing with an 8-character password limit.

CSRF protection is absent. Any authenticated user visiting an attacker-controlled page will execute state-changing requests (send mail, edit zones, delete data) without consent. Cookies lack Secure, HttpOnly, and SameSite attributes, enabling theft via JavaScript or cross-site inclusion.

Session fixation is possible because session IDs are not regenerated after authentication. An attacker can set a victim's session cookie before login, then hijack the session after successful authentication.

SQL injection exists where user input is concatenated directly into queries instead of using parameterized placeholders. HTML output escaping exists for mail content but is not applied consistently across all endpoints.

## Patterns

### Session Validation Pattern

Always validate sessions before processing requests in authenticated endpoints.

```
TSession session(cgi, "SneezyMUD");

if (!session.isValid()) {
    session.doLogin(cgi, "your-endpoint.cgi");
    return 0;
}

int account_id = session.getAccountID();
```

The isValid() check ensures the session exists in the database and has not expired. Failing this check redirects to login with a return URL. Never proceed with authenticated operations without this guard.

**Why:** Unauthenticated users can access any endpoint URL directly. Without session validation, they bypass authentication entirely.

### Authorization Check Pattern

After validating the session, check required wizard powers for builder tools.

```
if (!session.hasWizPower(POWER_BUILDER)) {
    cout << "Access denied - POWER_BUILDER required";
    return 0;
}
```

The hasWizPower() query checks if any character on the account has the specified power. This is account-level authorization, not character-level.

**Why:** Session validation only confirms identity. Authorization confirms permission for the specific operation.

### Safe SQL Query Pattern

Use TDatabase format specifiers to prevent SQL injection. Never concatenate user input directly into queries.

**DO:**
```
db.query("select * from player where name='%s'", user_input.c_str());
db.query("select * from obj where id=%i", object_id);
```

The %s specifier calls mysql_real_escape_string() on the argument. The %i specifier handles integers safely.

**DON'T:**
```
sstring query = "select * from player where name='" + user_input + "'";
db.query(query.c_str());
```

Direct concatenation bypasses escaping. An attacker can inject SQL commands through user_input.

**Why:** SQL injection allows arbitrary database access, modification, or deletion. It is a critical vulnerability.

### Raw String Specifier Danger

The %r specifier inserts strings without escaping. Only use it for server-generated SQL fragments, never user input.

**Safe use case:**
```
sstring getPlayerNames() {
    // Queries database, builds quoted list
    return "'grimhaven', 'brightmoon', 'shadowfax'";
}

db.query("select * from obj where owner in (%r)", getPlayerNames().c_str());
```

The string comes from the database, not user input, so it's trusted.

**Dangerous use case:**
```
sstring my_query = "SELECT ...";
my_query += " AND LOWER(i.name) = LOWER(CONCAT('ITEM_' ,'" + user_type + "'))";
db.query(my_query.c_str());
```

This concatenates user input (user_type) directly into the query string. Even though %r isn't explicitly used, the pattern is identical: untrusted input becomes part of the SQL command.

**Why:** %r means "I trust this string to be valid SQL." User input is never trustworthy.

### Account Ownership Verification

When operations target a specific player, verify the player belongs to the authenticated account.

**DO:**
```
db.query("delete mail from mail, player where mail.mailid=%i and "
         "mail.mailto=player.name and player.account_id=%i",
         mail_id, account_id);
```

The WHERE clause requires both the mail ID and account ownership. A user cannot delete another account's mail.

**DON'T:**
```
int player_id = convertTo<int>(**(cgi.getElement("player")));
db.query("select * from player where id=%i", player_id);
```

This accepts player_id from form input without verifying it belongs to the session's account. An attacker can enumerate other players' data by manipulating the form parameter.

**Why:** Session validation proves identity but doesn't restrict which data that identity can access. Without ownership checks, users can access arbitrary accounts' data.

### HTML Output Escaping

Escape all user-generated content before rendering in HTML to prevent XSS.

```
sstring escape_html(sstring content) {
    content.replace("&", "&amp;");
    content.replace("<", "&lt;");
    content.replace(">", "&gt;");
    return content;
}

cout << escape_html(mail_body);
```

**Why:** User input containing <script> tags can execute JavaScript in other users' browsers, enabling session theft or CSRF attacks.

### Anti-Pattern: Unauthenticated Endpoints

Some endpoints have no session check at all.

**corpinfo.cgi.cc:**
```
int main(int argc, char** argv) {
    Cgicc cgi;
    // Directly queries and displays corporation data
    form_iterator corp_id = cgi.getElement("corp_id");
    // ...
}
```

**limb_quest.cgi.cc:**
```
int main(int argc, char** argv) {
    char* query = getenv("QUERY_STRING");
    // Directly displays quest data for any team name
}
```

These expose game data to anyone with web access. If the data should be restricted, add session validation. If the data is intentionally public, document that decision explicitly.

### Anti-Pattern: Missing CSRF Tokens

Forms submit state-changing operations without CSRF protection.

```
cout << "<form method=post action=mudmail.cgi>" << endl;
cout << "<input type=hidden name=state value=sendmail>" << endl;
cout << "Recipient: <input name=recipient>" << endl;
cout << "<textarea name=body></textarea>" << endl;
cout << "<input type=submit value=Send>" << endl;
cout << "</form>" << endl;
```

An attacker can craft a malicious page that submits this form on behalf of an authenticated victim. When the victim visits the attacker's page, their browser includes their session cookie, and the mail is sent without their knowledge.

**Mitigation:** Generate a random token, store it in the session, include it as a hidden form field, and validate it on submission.

### Anti-Pattern: Weak Session Duration

Auto-login creates year-long sessions.

```
if (autologin == cgi.getElements().end()) {
    createSession();  // Default duration
} else {
    createSession(60 * 60 * 24 * 365);  // 1 year
}
```

Long-lived sessions increase the window for session hijacking. If a session ID is compromised through network sniffing, XSS, or browser history, the attacker has a year to exploit it.

**Recommendation:** Maximum 30-day sessions with periodic refresh.

### Anti-Pattern: Insecure Cookie Attributes

Cookies lack security flags.

```
cgicc::HTTPCookie cookie(cookiename, getSessionID());
if (cookieduration >= 0)
    cookie.setMaxAge(cookieduration);
// No Secure flag - cookie sent over HTTP
// No HttpOnly flag - JavaScript can read cookie
// No SameSite attribute - cross-site requests include cookie
```

Without Secure, cookies are transmitted over unencrypted HTTP, enabling network sniffing. Without HttpOnly, XSS attacks can steal cookies. Without SameSite, CSRF attacks include the cookie.

## Reference

### CGI Endpoints

| File | Purpose | Authentication | Authorization | Risk Level |
|------|---------|----------------|---------------|------------|
| session.cgi.cc | Session creation and login | N/A (provides auth) | N/A | CRITICAL |
| mudmail.cgi.cc | Player mail system | Session required | Account ownership | Medium |
| shopinfo.cgi.cc | Shop information | Session required | Account ownership | Medium |
| objeditor.cgi.cc | Object editing | Session required | POWER_BUILDER | Medium |
| roomeditor.cgi.cc | Room editing | Session required | POWER_BUILDER | Medium |
| respeditor.cgi.cc | Response editing | Session required | POWER_BUILDER | Medium |
| objlog.cgi.cc | Object load logging | Session required | POWER_BUILDER | HIGH (SQL injection) |
| eqcalc.cgi.cc | Equipment calculator | Session required | None | Low |
| corpinfo.cgi.cc | Corporation listings | NONE | N/A | Medium (info disclosure) |
| limb_quest.cgi.cc | Quest limb tracking | NONE | N/A | Medium (info disclosure) |

### TDatabase Query Specifiers

| Specifier | Type | Escaping | Use Case |
|-----------|------|----------|----------|
| %s | string | mysql_real_escape_string | User input, player names, any untrusted string |
| %i | integer | N/A (numeric conversion) | IDs, counts, numeric parameters |
| %r | raw string | NONE | Pre-built SQL fragments, server-generated lists ONLY |

### Wizard Powers (Authorization)

| Power | Purpose | Endpoints Requiring It |
|-------|---------|------------------------|
| POWER_BUILDER | Zone editing | objeditor, roomeditor, respeditor, objlog |
| POWER_LOAD | Object spawning | (varies by tool) |

### Session Table Schema

| Column | Type | Purpose |
|--------|------|---------|
| session_id | VARCHAR | MD5 hash of entropy sources |
| account_id | INT | Foreign key to account |
| duration | INT | Session lifetime in seconds |
| timeset | INT | Unix timestamp of creation |
| name | VARCHAR | Account name (denormalized) |

### Entropy Sources for Session ID

| Source | Value | Entropy Level | Attack Vector |
|--------|-------|---------------|---------------|
| time(NULL) | Current Unix timestamp | Very Low | Predictable to the second |
| random() | Unseeded PRNG | Very Low | Defaults to seed 1 if not initialized |
| getpid() | Process ID | Low | Observable via /proc, sequential allocation |
| &seed | Stack address | Low-Medium | Limited ASLR, predictable stack layout |

## Implementation

### Session Creation Flow

When a user submits the login form, sendLoginCheck() receives the CGI request. It extracts the account name and password from form parameters. The checkPasswd() function queries the account table for the account name, retrieves the stored password hash, and calls crypt() with the submitted password and stored hash as the salt. The crypt() output is truncated to 10 characters and compared to the stored hash. If they match, authentication succeeds.

On success, createSession() calls generateSessionID() to produce a new token. This function allocates a 16-byte array and four integer seed values. It sets seed[0] to the current time, seed[1] to random(), seed[2] to the process ID, and seed[3] to the address of the seed array itself. These four integers are copied into the data array. The array is passed to OpenSSL's MD5() function, which returns a 16-byte binary hash. The hash is converted to a 32-character hexadecimal string.

The session ID, account ID, duration (default or year-long if auto-login is checked), current timestamp, and account name are inserted into the cgisession table. An HTTPCookie object is created with the session ID and returned in the HTTP headers. The cookie has no security flags set.

### Session Validation Flow

Each authenticated endpoint instantiates a TSession object, passing the CGI request and a cookie name. The constructor searches for the cookie in the HTTP request headers. If found, it extracts the session ID and queries the cgisession table for a matching entry. The query retrieves the account_id, timeset, and duration columns. It calculates expiration as timeset + duration and compares to the current time. If the session is expired or not found, isValid() returns false. Otherwise, it stores the account_id and returns true.

After session validation, endpoints requiring builder access call hasWizPower() with the required power constant. This function queries the wizpower table joined with player and account tables to check if any character on the authenticated account has the specified power. It returns true if any match is found.

### Password Hashing Implementation

The checkPasswd() function uses the crypt() function from the C standard library. On most systems, crypt() defaults to DES-based hashing with a two-character salt. DES truncates passwords to 8 characters and produces a 13-character hash (2-character salt plus 11 characters of hash data). The code truncates the crypt() output to 10 characters before comparison, reducing the effective hash strength further.

This implementation predates modern password hashing algorithms like bcrypt, scrypt, and argon2, which include work factors, longer salts, and resistance to GPU cracking.

### SQL Query Execution

TDatabase provides a printf-style query interface. The query() method accepts a format string and variadic arguments. When it encounters %s, it calls mysql_real_escape_string() on the corresponding argument, which escapes special characters (single quote, double quote, backslash, NULL byte) according to MySQL's character set. The escaped string is inserted into the query. When it encounters %i, it converts the integer argument to a string and inserts it directly. When it encounters %r, it inserts the string argument without any modification.

The objlog.cgi.cc endpoint builds queries by string concatenation instead of using the format specifiers. It constructs a base query string, then appends user input directly:

```
sstring my_query = "SELECT obj.vnum, obj.name, ...";
my_query += " AND LOWER(i.name) = LOWER(CONCAT('ITEM_' ,'" + type + "'))";
```

The type variable comes from a CGI form element without sanitization. An attacker can submit type=' OR '1'='1 to inject SQL logic, or type='; DROP TABLE obj; -- to execute destructive commands.

### HTML Output Escaping

The escape_html() function in session.cgi.cc performs three string replacements: ampersand to &amp;, less-than to &lt;, greater-than to &gt;. This prevents user input from being interpreted as HTML tags or JavaScript. The mudmail.cgi.cc endpoint applies this function to mail body content before rendering. However, other endpoints do not consistently escape output, creating potential XSS vectors where user-controlled data is displayed.

### Account Ownership Verification Patterns

The mudmail delete operation joins the mail and player tables, requiring both the mail ID and that the mail recipient's account_id matches the session's account_id. This prevents users from deleting other accounts' mail by guessing or enumerating mail IDs.

Some endpoints accept a player_id form parameter to select which character to operate on (for accounts with multiple characters). Safe implementations query the player table with both the player_id and account_id:

```
db.query("select name from player where id=%i and account_id=%i",
         player_id, account_id);
```

This ensures the selected player belongs to the authenticated account. Unsafe implementations query only by player_id, allowing users to access arbitrary characters by manipulating the form parameter.

### Form Processing Patterns

CGI endpoints use the cgicc library to parse HTTP requests. The Cgicc object extracts form elements by name:

```
form_iterator recipient = cgi.getElement("recipient");
if (recipient != cgi.getElements().end()) {
    sstring recipient_name = **recipient;
}
```

The double-dereference (**) extracts the string value. This value is untrusted user input and must be validated or escaped before use in SQL queries, shell commands, or HTML output.

Integer form elements are converted with convertTo<int>():

```
int player_id = convertTo<int>(**(cgi.getElement("player")));
```

This handles non-numeric input by throwing an exception or returning zero, preventing crashes but not preventing logic errors if zero is a valid ID.

### Error Message Handling

Some endpoints reveal internal state through error messages. The mudmail.cgi.cc contains commented code with "joke" messages that reference internal authorization checks:

```
cout << "secret pwipe code received. full pwipe initiating in 3.. 2.. 1..";
```

While intended as humor, error messages that reveal the existence or failure of authorization checks provide reconnaissance information to attackers.

Production error messages should be generic to external users while logging detailed information internally for debugging.

### File Organization

All CGI executables are in code/cgi/ with .cc source files. Each endpoint is a standalone executable compiled and deployed to the web server's CGI directory. The TSession class is defined in session.cgi.h and implemented in session.cgi.cc, but it is compiled separately into each endpoint that uses it, not as a shared library. The TDatabase class is defined in code/sys/database.h and provides the query interface used by all endpoints.

Session data is stored in the sneezy database's cgisession table. Account and player data is in the same database. Wizard powers are in the wizpower table. The database schema is defined in _Setup-data/sql_tables/.

## Troubleshooting

### Symptom: User Session Lost Unexpectedly

**Likely cause:** Session expired based on timeset + duration calculation, or session ID not preserved in cookie.

**Diagnostic approach:**
1. Check if session exists in cgisession table with the session ID from the cookie
2. Compare timeset + duration to current time
3. Verify cookie is being sent in HTTP headers (browser developer tools)
4. Check if cookie domain/path matches the request URL

**Fix:** If sessions expire too quickly, increase duration. If cookies are not being sent, verify cookie attributes match the deployment environment (path, domain).

### Symptom: "Access Denied" on Builder Tools

**Likely cause:** Account lacks required wizard power, or session validation failed before power check.

**Diagnostic approach:**
1. Verify session is valid (not expired)
2. Query wizpower table joined with player and account to check if any character on the account has the required power
3. Check if hasWizPower() is using the correct power constant

**Fix:** Grant the required wizard power to at least one character on the account using the in-game immortal commands.

### Symptom: SQL Injection Exploit Suspected

**Likely cause:** User input concatenated directly into query instead of using %s or %i specifiers.

**Diagnostic approach:**
1. Identify which endpoint is vulnerable by checking web server logs for unusual SQL errors
2. Search the endpoint's source for string concatenation with user input (+ operator with CGI form elements)
3. Look for %r specifier usage with untrusted input

**Fix:** Rewrite query to use %s for string parameters and %i for integer parameters. Remove direct concatenation.

### Symptom: XSS Attack Displaying JavaScript

**Likely cause:** User-generated content displayed in HTML without escaping.

**Diagnostic approach:**
1. Identify which endpoint renders the malicious content
2. Check if escape_html() is called on the content before output
3. Test by submitting <script>alert(1)</script> as input

**Fix:** Apply escape_html() to all user-controlled content before rendering in HTML.

### Symptom: Session Hijacking After Logout

**Likely cause:** Session not deleted from database on logout, or session ID predictable.

**Diagnostic approach:**
1. Check if logout handler deletes the session from cgisession table
2. Test session ID predictability by creating multiple sessions and analyzing the patterns
3. Verify cookie is cleared in HTTP headers after logout

**Fix:** Delete session from database on logout. Regenerate session ID on authentication. Implement cryptographically secure random session ID generation.

### Symptom: Users Accessing Other Accounts' Data

**Likely cause:** Missing account ownership verification in query.

**Diagnostic approach:**
1. Identify which endpoint is leaking data
2. Check if SQL queries include account_id in WHERE clause
3. Test by manipulating form parameters (player_id, mail_id) to access other accounts' data

**Fix:** Add account_id to WHERE clause in all queries that access player-specific data. Join with player table to verify ownership.

### Symptom: CSRF Attack Executing Unwanted Actions

**Likely cause:** No CSRF token validation.

**Diagnostic approach:**
1. Check if form includes a CSRF token field
2. Check if form handler validates the token against session
3. Test by crafting an external page that submits the form

**Fix:** Generate random token on form render, store in session, include as hidden field, validate on submission.
