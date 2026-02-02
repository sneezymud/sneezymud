---
title: CGI Web Interface Security
category: critical
created_by_model: opus
keywords: [TSession, session, CGI, authentication, SQL injection, CSRF, cookies]
related: []
primary_symbols:
  functions: [generateSessionID, checkPasswd, hasWizPower, createSession, escape_html]
  classes: [TSession, TDatabase, Cgicc]
  files: [code/code/cgi/session.cgi.cc, code/code/cgi/session.cgi.h, code/code/cgi/mudmail.cgi.cc, code/code/cgi/objeditor.cgi.cc, code/code/cgi/objlog.cgi.cc, code/code/cgi/corpinfo.cgi.cc, code/code/cgi/limb_quest.cgi.cc, code/code/sys/database.h]
---

## Overview

How does a MUD provide browser-based utilities while protecting player accounts and builder data from attackers?

The CGI web interface exposes player utilities (mail, shop info) and builder tools (object/room/response editors) through traditional CGI applications. The `TSession` class manages authentication by storing session tokens in the database and validating them on each request. Builder tools additionally require wizard powers, checked via `hasWizPower()` against the account's character permissions.

This system presents substantial attack surface. Session tokens are generated with weak entropy, making them potentially predictable. Several endpoints lack authentication entirely, exposing game data to unauthenticated users. At least one endpoint concatenates user input directly into SQL queries, creating injection vulnerabilities. Cookies lack modern security attributes, and no CSRF protection exists.

When a player logs in, they submit credentials to `session.cgi.cc`. The system validates passwords using legacy `crypt()`, generates a session ID, stores it in the `cgisession` table, and returns it as a cookie. Subsequent requests validate the session by checking the cookie against the database and verifying the session hasn't expired. For builder tools, the system additionally queries the `wizpower` table to verify the account has appropriate administrative privileges.

## Patterns

### Session Validation

Always call `isValid()` before using session data. Invalid sessions indicate expired or tampered tokens.

Never trust session data without database verification. The session ID in the cookie must match a valid, unexpired record in `cgisession`.

### Database Query Safety

Always use `%s` for string parameters in `TDatabase` queries. This invokes `mysql_real_escape_string` for proper escaping.

Always use `%i` for integer parameters. This provides type-safe numeric handling.

Never use `%r` with user input. The `%r` specifier passes strings through without escaping and exists only for server-generated SQL fragments like pre-built column lists. Using it with user-controlled data creates SQL injection vulnerabilities.

Never concatenate user input directly into query strings. Build queries using format specifiers exclusively.

### Authentication Requirements

Always verify session validity before processing requests in authenticated endpoints. Call `session.isValid()` and redirect to login on failure.

Always check `hasWizPower()` for builder tools. Verify `POWER_BUILDER` or the appropriate power level before allowing access to editing functions.

Never create endpoints that query or modify data without authentication. Information disclosure of game state (corporations, quests, player data) enables reconnaissance for other attacks.

### Account Ownership Verification

Always verify account ownership when processing player-specific requests. When accepting a `player_id` parameter, confirm the player belongs to the authenticated account before querying or modifying data.

### Cookie Security

Always set the `Secure` flag on session cookies. This prevents transmission over unencrypted connections.

Always set the `HttpOnly` flag. This prevents JavaScript from accessing the cookie, mitigating XSS token theft.

Always set `SameSite` to `Strict` or `Lax`. This prevents cross-site request inclusion attacks.

### CSRF Prevention

Always include a unique, unguessable token in forms. Validate the token on form submission before processing the request.

Never rely solely on session cookies for request validation. Cookies are automatically included in cross-site requests.

### Password Security

Never truncate password hashes. The current system truncates to 10 characters, drastically reducing cryptographic strength.

Never use legacy `crypt()` for new password storage. It defaults to DES, which limits passwords to 8 characters and is computationally trivial to crack.

### Session Generation

Never use predictable values as entropy sources. Current generation uses `time(NULL)`, `random()` (unseeded), `getpid()`, and stack addresses, all of which are guessable or have limited entropy.

Always use cryptographic random sources for session IDs. Read from `/dev/urandom` or use OpenSSL's `RAND_bytes()`.

### Error Messages

Never reveal internal state in error messages. Messages should be generic enough that attackers cannot infer implementation details.

### Session Duration

Never allow year-long session durations. The auto-login feature creates sessions lasting 365 days, providing extensive windows for session hijacking.

## Reference

### Primary Symbols

| Symbol | Type | Purpose |
|--------|------|---------|
| `TSession` | class | Manages authentication state and session lifecycle |
| `TDatabase` | class | Database abstraction with format-specifier escaping |
| `Cgicc` | class | CGI request/response library (external) |
| `generateSessionID()` | function | Creates session token (weak implementation) |
| `checkPasswd()` | function | Validates credentials against database |
| `hasWizPower()` | function | Checks account for administrative privileges |
| `createSession()` | function | Stores session in database and sets cookie |
| `escape_html()` | function | Escapes HTML special characters for XSS prevention |

### CGI Endpoints

| File | Purpose | Auth Required | Risk |
|------|---------|---------------|------|
| `session.cgi.cc` | Login and session management | N/A | Critical |
| `mudmail.cgi.cc` | Player mail system | Session | Medium |
| `shopinfo.cgi.cc` | Player shop information | Session | Medium |
| `objeditor.cgi.cc` | Object editing | POWER_BUILDER | Medium |
| `roomeditor.cgi.cc` | Room editing | POWER_BUILDER | Medium |
| `respeditor.cgi.cc` | Response editing | POWER_BUILDER | Medium |
| `objlog.cgi.cc` | Object load logging | POWER_BUILDER | High |
| `eqcalc.cgi.cc` | Equipment calculator | Session | Low |
| `corpinfo.cgi.cc` | Corporation listings | **None** | Medium |
| `limb_quest.cgi.cc` | Quest limb tracking | **None** | Medium |

### TDatabase Format Specifiers

| Specifier | Type | Escaping | Safety |
|-----------|------|----------|--------|
| `%s` | string | `mysql_real_escape_string` | Safe for user input |
| `%i` | integer | Numeric conversion | Safe |
| `%r` | raw string | None | Unsafe for user input |

### Session Table Schema

| Column | Type | Purpose |
|--------|------|---------|
| `session_id` | VARCHAR | Token for cookie validation |
| `account_id` | INT | Owner account reference |
| `duration` | INT | Session lifetime in seconds |
| `timeset` | INT | Creation timestamp |
| `name` | VARCHAR | Associated player name |

### Known Vulnerabilities

| Vulnerability | Location | Severity |
|---------------|----------|----------|
| Weak session ID entropy | `generateSessionID()` | Critical |
| SQL injection | `objlog.cgi.cc` | Critical |
| Unauthenticated endpoints | `corpinfo.cgi.cc`, `limb_quest.cgi.cc` | Medium |
| No CSRF protection | All forms | Medium |
| Missing cookie security flags | `TSession` cookie handling | Medium |
| Password hash truncation | `checkPasswd()` | Medium |
| Legacy crypt() algorithm | `checkPasswd()` | Medium |
| Year-long session duration | Auto-login feature | Low |

## Implementation

### Session Lifecycle

Session creation begins when the user submits credentials to the login form. The `sendLoginCheck()` method receives the form submission and calls `checkPasswd()` to validate credentials. Password validation queries the database for the account record, then uses `crypt()` to compare the submitted password against the stored hash. The implementation truncates the result to 10 characters before comparison.

Upon successful authentication, `createSession()` generates a session ID via `generateSessionID()`. This function builds a 16-byte array from four 32-bit values: current Unix timestamp, a call to `random()`, the process ID, and the stack address of the local array. These values are hashed with MD5 and converted to a hex string. The resulting session ID is inserted into the `cgisession` table along with the account ID, duration, and current timestamp.

The session cookie is created using the cgicc library's `HTTPCookie` class. Only the `MaxAge` attribute is set; `Secure`, `HttpOnly`, and `SameSite` are not configured.

Session validation in `isValid()` queries `cgisession` for a matching session ID, then verifies that `timeset + duration` exceeds the current time. If valid, the account ID is cached for use by other methods.

The `hasWizPower()` method checks administrative privileges by querying the `wizpower` table. It looks for any character on the account that possesses the requested power flag.

### Database Query Handling

The `TDatabase` class provides a `query()` method accepting a format string and variadic arguments. The format string uses custom specifiers to handle escaping.

For `%s` specifiers, the implementation calls `mysql_real_escape_string()` on the argument before interpolation. This escapes single quotes, backslashes, and other special characters that could alter query structure.

For `%i` specifiers, the argument is converted to an integer, eliminating any non-numeric content.

The `%r` specifier performs no transformation, inserting the argument verbatim. This exists for server-generated fragments like pre-built value lists. Safe usage requires the string to never contain user input.

The SQL injection vulnerability in `objlog.cgi.cc` arises from string concatenation rather than format specifiers. The `type` parameter from the form is concatenated directly into the query string with `+` operators, bypassing `TDatabase` escaping entirely.

### HTML Output Escaping

The `escape_html()` function in `session.cgi.cc` transforms HTML-significant characters. It replaces `&` with `&amp;`, `<` with `&lt;`, and `>` with `&gt;`. This prevents user-supplied content from injecting HTML tags or scripts.

The mail system uses this function when displaying message content and sender names, preventing XSS attacks through malicious mail.

### Unauthenticated Endpoints

Two CGI applications skip session validation entirely.

`corpinfo.cgi.cc` accepts a `corp_id` parameter and directly queries corporation data for display. No `TSession` object is created, and no authentication check exists.

`limb_quest.cgi.cc` reads the `QUERY_STRING` environment variable directly rather than using the cgicc library. It extracts a team name, performs minimal sanitization (replacing `+` with space), and queries quest data. No session validation occurs.

### Auto-Login Duration

The login form includes an optional auto-login checkbox. When checked, `createSession()` receives a duration argument of `60 * 60 * 24 * 365` (one year in seconds). Standard sessions receive no explicit duration argument, using a shorter default.

### Account Ownership Patterns

Mail deletion in `mudmail.cgi.cc` demonstrates correct ownership verification. The delete query joins the mail and player tables, requiring both the mail ID to match and the player's account ID to match the session's account ID.

Some endpoints accept a `player_id` form parameter without fully verifying ownership. The player ID is converted from form input and used in queries that should verify account membership but may not consistently do so.

### Builder Tool Authorization

Builder endpoints follow a common pattern after session validation. After confirming `isValid()`, they call `hasWizPower(POWER_BUILDER)` to verify the account has builder privileges. Only if this check passes does the application proceed to display or modify data.

The wizard power check queries the `wizpower` table, searching for any character on the session's account that has the requested power flag set.

## Troubleshooting

### Session Validation Failures

**Symptom:** User is redirected to login despite having logged in recently.

**Likely cause:** Session expired, cookie not sent, or session record missing from database.

**Diagnostic approach:** Check the `cgisession` table for a record matching the session ID. Verify `timeset + duration` against current time. Check browser cookie storage for the session cookie.

**Fix:** If sessions expire too quickly, verify the duration parameter passed to `createSession()`. If cookies aren't sent, check that the cookie domain and path match the request.

### Authorization Denied for Builder

**Symptom:** Builder receives "Access denied" despite having builder privileges in-game.

**Likely cause:** The `wizpower` table lacks the `POWER_BUILDER` entry for any character on the account.

**Diagnostic approach:** Query `wizpower` for the account's character IDs. Verify the power flags include `POWER_BUILDER`.

**Fix:** Grant `POWER_BUILDER` to a character on the account through the in-game administrative commands.

### SQL Errors in Query Execution

**Symptom:** Database query fails with syntax errors.

**Likely cause:** Incorrect format specifier usage or unescaped input reaching the query.

**Diagnostic approach:** Enable query logging to see the actual SQL being executed. Check for unescaped quotes or semicolons in the interpolated query.

**Fix:** Ensure all user input uses `%s` specifier. Never concatenate user input directly into query strings.

### XSS Content Displayed

**Symptom:** HTML tags or scripts execute when viewing user-generated content.

**Likely cause:** Content output without `escape_html()` processing.

**Diagnostic approach:** Trace the display path for the affected content. Verify `escape_html()` is called before output.

**Fix:** Apply `escape_html()` to all user-generated content before HTML output.
