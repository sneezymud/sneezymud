---
title: CGI Web Interface Security
description: The CGI web interface provides browser-based utilities for players and builders with significant security weaknesses including weak session ID generation, no CSRF protection, and unauthenticated endpoints.
keywords: [TSession, generateSessionID, checkPasswd, hasWizPower, TDatabase query, SQL injection, CSRF protection, session fixation, cookie security, mysql_real_escape_string, cgisession table, escape_html]
category: Critical Systems
related: []
last_updated: 2026-01-29
source_files: [code/code/cgi/session.cgi.cc, code/code/cgi/session.cgi.h, code/code/cgi/mudmail.cgi.cc, code/code/cgi/shopinfo.cgi.cc, code/code/cgi/objeditor.cgi.cc, code/code/cgi/roomeditor.cgi.cc, code/code/cgi/respeditor.cgi.cc, code/code/cgi/objlog.cgi.cc, code/code/cgi/eqcalc.cgi.cc, code/code/cgi/corpinfo.cgi.cc, code/code/cgi/limb_quest.cgi.cc, code/code/sys/database.h]
---

# CGI Web Interface Security

The CGI web interface provides browser-based utilities for players and builders. This document describes the security architecture, known vulnerabilities, and recommendations.

**WARNING:** This system has significant security weaknesses including weak session ID generation, no CSRF protection, unauthenticated endpoints, and potential SQL injection vectors. It should be considered for deprecation or major refactoring.

## CGI Utilities Overview

| File | Purpose | Authentication | Risk Level |
|------|---------|----------------|------------|
| `session.cgi.cc` | Core session/login handling | N/A (provides auth) | **CRITICAL** |
| `mudmail.cgi.cc` | Player mail system | Session required | Medium |
| `shopinfo.cgi.cc` | Player shop information | Session required | Medium |
| `objeditor.cgi.cc` | Object editing for builders | POWER_BUILDER | Medium |
| `roomeditor.cgi.cc` | Room editing for builders | POWER_BUILDER | Medium |
| `respeditor.cgi.cc` | Response editing for builders | POWER_BUILDER | Medium |
| `objlog.cgi.cc` | Object load logging | POWER_BUILDER | **HIGH** |
| `eqcalc.cgi.cc` | Equipment calculator | Session required | Low |
| `corpinfo.cgi.cc` | Corporation listings | **NONE** | Medium |
| `limb_quest.cgi.cc` | Quest limb tracking | **NONE** | Medium |

**Source:** `/code/code/cgi/`

## Session Management

### Session Creation Flow

1. User submits account name and password via login form
2. `TSession::checkPasswd()` validates credentials against database
3. `TSession::createSession()` generates session ID and stores in `cgisession` table
4. Session ID returned as HTTP cookie

### Session ID Generation - CRITICAL WEAKNESS

```cpp
// session.cgi.cc - generateSessionID()
sstring TSession::generateSessionID() {
  unsigned char data[16];
  int seed[4];

  seed[0] = time(NULL);      // Predictable: current timestamp
  seed[1] = random();        // Weak: unseeded PRNG
  seed[2] = getpid();        // Predictable: process ID
  seed[3] = (int)&seed;      // Weak: stack address

  // ... copies to data array, then:
  sstring md5 = (char*)MD5(data, length, NULL);
  // ... converts to hex string
}
```

**Vulnerabilities:**
- `time(NULL)` - Attacker can narrow to seconds around login time
- `random()` - No `srandom()` call visible; uses default seed
- `getpid()` - Guessable from `/proc` or sequential allocation
- `&seed` - Stack address has limited entropy, especially with ASLR disabled

**Impact:** Session tokens may be predictable, enabling session hijacking.

### Session Storage

```sql
-- cgisession table
session_id VARCHAR,
account_id INT,
duration INT,
timeset INT,
name VARCHAR
```

Session validated by checking `(timeset + duration) > current_time`.

### Password Handling - WEAKNESS

```cpp
// session.cgi.cc - checkPasswd()
sstring crypted = crypt(passwd.c_str(), db["passwd"].c_str());
crypted = crypted.substr(0, 10);  // Truncate to 10 characters!

if (crypted != db["passwd"])
    return false;
```

**Problem:** Password hash truncated to 10 characters, reducing cryptographic strength. Uses legacy `crypt()` function (DES-based by default, 8-character password limit).

## Authentication Mechanisms

### Session-Based Authentication

```cpp
TSession session(cgi, "SneezyMUD");

if (!session.isValid()) {
    session.doLogin(cgi, "mudmail.cgi");
    return 0;
}
// User is authenticated
int account_id = session.getAccountID();
```

### Wizard Power Checks

Builder tools verify administrative privileges:

```cpp
if (!session.hasWizPower(POWER_BUILDER)) {
    cout << "Access denied - POWER_BUILDER required";
    return 0;
}
```

`hasWizPower()` queries the `wizpower` table to check if any character on the account has the required power.

### Unauthenticated Endpoints - VULNERABILITY

**corpinfo.cgi.cc** - No session check at all:

```cpp
int main(int argc, char** argv) {
    Cgicc cgi;
    // No authentication!
    form_iterator corp_id = cgi.getElement("corp_id");
    // Directly queries and displays corporation data
}
```

**limb_quest.cgi.cc** - No session check:

```cpp
int main(int argc, char** argv) {
    char* query = getenv("QUERY_STRING");
    // No authentication!
    // Directly displays quest data for any team name
}
```

**Risk:** Information disclosure of corporation and quest data to unauthenticated users.

## SQL Query Patterns

### Safe Patterns (TDatabase)

The `TDatabase` class provides format specifiers that handle escaping:

| Specifier | Type | Escaping | Example |
|-----------|------|----------|---------|
| `%s` | string | `mysql_real_escape_string` | `"name='%s'"` |
| `%i` | integer | N/A (numeric) | `"id=%i"` |
| `%r` | raw string | **NONE** | For pre-built SQL fragments |

**Safe usage examples:**

```cpp
// Integer parameter - safe
db.query("select * from player where id=%i", player_id);

// String parameter - escaped
db.query("select * from player where name='%s'", name.c_str());

// Array of quoted values - safe when server-generated
sstring owners = getPlayerNames();  // Returns "'name1', 'name2'"
db.query("select * from obj where owner in (%r)", owners.c_str());
```

### DANGEROUS Pattern - objlog.cgi.cc

```cpp
// objlog.cgi.cc - SQL INJECTION VULNERABILITY
sstring my_query = "SELECT ...";
my_query += " AND LOWER(i.name) = LOWER(CONCAT('ITEM_' ,'" + (**type) + "'))";
// User input concatenated directly into query!
```

**Impact:** Arbitrary SQL execution if attacker controls `type` parameter.

### Player Name Array Pattern

Builder tools use `%r` with server-generated player lists:

```cpp
// objeditor.cgi.cc
sstring getPlayerNames() {
    // Builds quoted, comma-separated list from database
    return "'grimhaven', 'brightmoon', ...";
}

db.query("select * from obj where owner in (%r)", players.c_str());
```

This is safe because the list comes from the database, not user input.

## Input Validation

### HTML Output Escaping

```cpp
// session.cgi.cc
sstring escape_html(sstring content) {
    // Escapes &, <, > for safe HTML display
    content.replace("&", "&amp;");
    content.replace("<", "&lt;");
    content.replace(">", "&gt;");
    return content;
}
```

Used in mudmail to prevent XSS when displaying mail content.

### Integer Conversion

```cpp
int player_id = convertTo<int>(**(cgi.getElement("player")));
```

Safe for SQL but may throw on invalid input.

### Minimal Validation - limb_quest.cgi.cc

```cpp
char* query = getenv("QUERY_STRING");
for (size_t i = 0; i < team.size(); i++) {
    if (team[i] == '+')
        team[i] = ' ';
}
// Only replaces + with space, no other sanitization
```

**Risk:** Insufficient validation of team name parameter.

## Known Security Gaps

### No CSRF Protection

Forms lack CSRF tokens. Example from mudmail:

```cpp
cout << "<form method=post action=mudmail.cgi>" << endl;
cout << "<input type=hidden name=state value=sendmail>" << endl;
// No CSRF token!
```

**Impact:** Attackers can forge requests on behalf of authenticated users.

### Session Fixation

No session regeneration after authentication:

```cpp
void TSession::sendLoginCheck(Cgicc cgi, sstring url) {
    if (!checkPasswd(name, passwd)) { ... }
    createSession();  // New session created
    // But no check for pre-existing session ID
}
```

### Cookie Security

```cpp
cgicc::HTTPCookie cookie(cookiename, getSessionID());
if (cookieduration >= 0)
    cookie.setMaxAge(cookieduration);
// No Secure flag
// No HttpOnly flag
// No SameSite attribute
```

**Missing protections:**
- `Secure` - Cookie sent over HTTPS only
- `HttpOnly` - Cookie inaccessible to JavaScript
- `SameSite` - Prevents cross-site request inclusion

### Information Disclosure

Error messages reveal internal state:

```cpp
// mudmail.cgi.cc
cout << "secret pwipe code received. full pwipe initiating in 3.. 2.. 1..";
```

This "joke" error message appears when authorization checks fail, but reveals that authorization checks exist and failed.

### Auto-login Duration

```cpp
if (autologin == cgi.getElements().end()) {
    createSession();
} else {
    createSession(60 * 60 * 24 * 365);  // 1 year session!
}
```

Year-long sessions increase window for session hijacking.

## Authorization Bypass Risks

### Account-Based Access Control

Mail and shop access verified by account ID:

```cpp
// mudmail.cgi.cc - deleteMessage
db.query("delete mail from mail, player where mail.mailid=%i and "
         "mail.mailto=player.name and player.account_id=%i",
         mail_id, account_id);
```

This is correct - deletion requires account ownership. However, the pattern must be followed consistently.

### Player Selection Vulnerability

```cpp
// mudmail.cgi.cc - sendMessageList
int player_id = convertTo<int>(**(cgi.getElement("player")));
// Query uses player_id but should verify account ownership
db.query("select name from player where id=%i", player_id);
```

Some endpoints accept player_id from form input but may not verify the player belongs to the logged-in account.

## Recommendations

### Critical Priority

1. **Deprecate or rewrite session generation** - Use cryptographically secure random bytes (`/dev/urandom` or OpenSSL RAND_bytes)

2. **Fix SQL injection in objlog.cgi.cc** - Use parameterized queries with `%s` specifier

3. **Add authentication to public endpoints** - corpinfo.cgi.cc and limb_quest.cgi.cc should require session

### High Priority

4. **Implement CSRF protection** - Add tokens to all forms, validate on submission

5. **Set secure cookie attributes**:
   ```cpp
   cookie.setSecure(true);
   cookie.setHttpOnly(true);
   cookie.setSameSite("Strict");
   ```

6. **Audit all player_id usage** - Ensure account ownership verification

### Medium Priority

7. **Upgrade password hashing** - Migrate from crypt() to bcrypt/scrypt/argon2

8. **Reduce auto-login duration** - 30 days maximum, with session refresh

9. **Add rate limiting** - Prevent brute force attacks on login

10. **Sanitize error messages** - Remove internal details from user-facing errors

### Consider Deprecation

The CGI interface uses legacy technology (cgicc library, raw CGI) and has accumulated security debt. Consider:

- Migrating to a modern web framework with built-in security features
- Implementing a REST API with proper authentication (JWT/OAuth)
- Using the game server itself for web requests via embedded HTTP server

## Related Files

| File | Purpose |
|------|---------|
| `/code/code/cgi/*.cc` | CGI application implementations |
| `/code/code/cgi/session.cgi.h` | TSession class declaration |
| `/code/code/sys/database.h` | TDatabase query interface |
| `/_Setup-data/sql_tables/sneezy/cgisession.sql` | Session table schema |
