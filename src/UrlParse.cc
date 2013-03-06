/**
* Copyright (c) 2011-2012 Pangoso
* @version 1.0.0
* @author Zhai Zhouwei
* @email zhaizhouwei@panguso.com
* @date 2012-4-26
* @description URL分析处理
*/
#include "UrlParse.h"

#include <stdlib.h>
#include <iostream>

bool IsAuthorityTerminator(char16 ch) {
  return IsURLSlash(ch) || ch == '?' || ch == '#';
}

int FindNextAuthorityTerminator(const char* spec,int start_offset,int spec_len) 
    {  
	for (int i = start_offset; i < spec_len; i++) {    
	if (IsAuthorityTerminator(spec[i]))      
		return i;  
	}
	return spec_len;  // Not found.
    }
void ParseUserInfo(const char* spec,const Component& user,Component* username,Component* password) 
    {  // Find the first colon in the user section, which separates the username and password.  
	int colon_offset = 0;  
	while (colon_offset < user.len && spec[user.begin + colon_offset] != ':')    
	    colon_offset++;  
	if (colon_offset < user.len) 
	{    // Found separator: <username>:<password>    
	    *username = Component(user.begin, colon_offset); 
            *password = MakeRange(user.begin + colon_offset + 1,user.begin + user.len);  
        } 
        else 
        {    // No separator, treat everything as the username    
	    *username = user;
            *password = Component(); 
        }
    }
void ParseServerInfo(const char* spec,const Component& serverinfo,Component* hostname,Component* port_num) 
{  
     if (serverinfo.len == 0) {
     // No server info, host name is empty.
      hostname->reset();
      port_num->reset();
      return;
     }

    int ipv6_terminator = spec[serverinfo.begin] == '[' ? serverinfo.end() : -1;
    int colon = -1;
    // Find the last right-bracket, and the last colon.
    for (int i = serverinfo.begin; i < serverinfo.end(); i++) {
     switch (spec[i]) {
       case ']':
         ipv6_terminator = i;
         break;
       case ':':
         colon = i;
         break;
     }
   }

    if (colon > ipv6_terminator) {
     // Found a port number: <hostname>:<port>
     *hostname = MakeRange(serverinfo.begin, colon);
     if (hostname->len == 0)
       hostname->reset();
     *port_num = MakeRange(colon + 1, serverinfo.end());
   } 
   else {
    // No port: <hostname>
     *hostname = serverinfo;
     port_num->reset();
   } 
}

void DoParseAuthority(const char* spec,const Component& auth,Component* username,
                      Component* password,Component* hostname,Component* port_num) 
    {
   	//DCHECK(auth.is_valid()) << "We should always get an authority";
   	if (auth.len == 0) {
    	    username->reset();
    	    password->reset();
    	    hostname->reset();
    	    port_num->reset();
            return;
        }

  // Search backwards for @, which is the separator between the user info and
  // the server info.
 	int i = auth.begin + auth.len - 1;
  	while (i > auth.begin && spec[i] != '@')
    	i--;
  	if (spec[i] == '@') {
          // Found user info: <user-info>@<server-info>
    	  ParseUserInfo(spec, Component(auth.begin, i - auth.begin),username, password);
    	  ParseServerInfo(spec, MakeRange(i + 1, auth.begin + auth.len),hostname, port_num);
        } 
	else {
        // No user info, everything is server info.
        username->reset();
        password->reset();
        ParseServerInfo(spec, auth, hostname, port_num);
        }
    }
void ParsePath(const char* spec,const Component& path,Component* filepath,Component* query,Component* ref) 
   {
    // path = [/]<segment1>/<segment2>/<...>/<segmentN>;<param>?<query>#<ref>
    // Special case when there is no path.
    if (path.len == -1) {
     filepath->reset();
     query->reset();
     ref->reset();
     return;
    }
  //DCHECK(path.len > 0) << "We should never have 0 length paths"
  // Search for first occurrence of either ? or #.
   int path_end = path.begin + path.len;
   int query_separator = -1;  // Index of the '?'
   int ref_separator = -1;    // Index of the '#'
   for (int i = path.begin; i < path_end; i++) {
    switch (spec[i]) {
      case '?':
        // Only match the query string if it precedes the reference fragment
        // and when we haven't found one already.
        if (ref_separator < 0 && query_separator < 0)
          query_separator = i;
        break;
      case '#':
        // Record the first # sign only.
        if (ref_separator < 0)
          ref_separator = i;
        break;
     }
   }
  // Markers pointing to the character after each of these corresponding
  // components. The code below words from the end back to the beginning,
  // and will update these indices as it finds components that exist.
  int file_end, query_end;
  // Ref fragment: from the # to the end of the path.
  if (ref_separator >= 0) {
    file_end = query_end = ref_separator;
    *ref = MakeRange(ref_separator + 1, path_end);
  } 
  else {
    file_end = query_end = path_end;
    ref->reset();
  }
  // Query fragment: everything from the ? to the next boundary (either the end
  // of the path or the ref fragment).
  if (query_separator >= 0) {
    file_end = query_separator;
    *query = MakeRange(query_separator + 1, query_end);
  } else {
    query->reset();
  }
  // File path: treat an empty file path as no file path.
  if (file_end != path.begin)
    *filepath = MakeRange(path.begin, file_end);
  else
    filepath->reset();
}

bool DoExtractScheme(const char* url,int url_len,Component* scheme) 
{
  // Skip leading whitespace and control characters.
  int begin = 0;
  while (begin < url_len && ShouldTrimFromURL(url[begin]))
    begin++;
  if (begin == url_len)
    return false;  // Input is empty or all whitespace.

  // Find the first colon character.
  for (int i = begin; i < url_len; i++) {
    if (url[i] == ':') {
      *scheme = MakeRange(begin, i);
      return true;
    }
  }
  return false;  // No colon found: no scheme
}

// Fills in all members of the Parsed structure except for the scheme.
//
// |spec| is the full spec being parsed, of length |spec_len|.
// |after_scheme| is the character immediately following the scheme (after the
// colon) where we'll begin parsing.
void DoParseAfterScheme(const char* spec,int spec_len,int after_scheme,Parsed* parsed) 
{
    int num_slashes = CountConsecutiveSlashes(spec, after_scheme, spec_len);
    int after_slashes = after_scheme + num_slashes;

  // First split into two main parts, the authority (username, password, host,
  // and port) and the full path (path, query, and reference).
    Component authority;
    Component full_path;

  // Found "//<some data>", looks like an authority section. Treat everything
  // from there to the next slash (or end of spec) to be the authority. Note
  // that we ignore the number of slashes and treat it as the authority.
   int end_auth = FindNextAuthorityTerminator(spec, after_slashes, spec_len);
   authority = Component(after_slashes, end_auth - after_slashes);

   if (end_auth == spec_len)  // No beginning of path found.
    full_path = Component();
   else  // Everything starting from the slash to the end is the path.
    full_path = Component(end_auth, spec_len - end_auth);

  // Now parse those two sub-parts.
   DoParseAuthority(spec, authority, &parsed->username, &parsed->password,&parsed->host, &parsed->port);
   ParsePath(spec, full_path, &parsed->path, &parsed->query, &parsed->ref);
}
// The main parsing function for standard URLs. Standard URLs have a scheme,
// host, path, etc.
void DoParseStandardURL(const char* spec, int spec_len, Parsed* parsed) 
{
  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  TrimURL(spec, &begin, &spec_len);
  int after_scheme;
  if (DoExtractScheme(spec, spec_len, &parsed->scheme)) {
     after_scheme = parsed->scheme.end() + 1;  // Skip past the colon.
  } else {
    // Say there's no scheme when there is no colon. We could also say that
    // everything is the scheme. Both would produce an invalid URL, but this way
    // seems less wrong in more cases.
    parsed->scheme.reset();
    after_scheme = begin;
  }
  DoParseAfterScheme(spec, spec_len, after_scheme, parsed);
}

int DoParsePort(const char* spec, const Component& component) 
{
  // Easy success case when there is no port.
  const int kMaxDigits = 5;
  if (!component.is_nonempty())
    return PORT_UNSPECIFIED;

  // Skip over any leading 0s.
  Component digits_comp(component.end(), 0);
  for (int i = 0; i < component.len; i++) {
    if (spec[component.begin + i] != '0') 
     {
      digits_comp = MakeRange(component.begin + i, component.end());
      break;
     }
  }
  if (digits_comp.len == 0)
    return 0;  // All digits were 0.
  // Verify we don't have too many digits (we'll be copying to our buffer so
  // we need to double-check).
  if (digits_comp.len > kMaxDigits)
    return PORT_INVALID;

  // Copy valid digits to the buffer.
  char digits[kMaxDigits + 1];  // +1 for null terminator
  for (int i = 0; i < digits_comp.len; i++) {
    char ch = spec[digits_comp.begin + i];
    if (!IsPortDigit(ch)) {
      // Invalid port digit, fail.
      return PORT_INVALID;
    }
    digits[i] = static_cast<char>(ch);
  }
  // Null-terminate the string and convert to integer. Since we guarantee
  // only digits, atoi's lack of error handling is OK.
  digits[digits_comp.len] = 0;
  int port = atoi(digits);
  if (port > 65535)
    return PORT_INVALID;  // Out of range.
  return port;
}

void DoExtractFileName(const char* spec,const Component& path,Component* file_name) 
{
  // Handle empty paths: they have no file names.
  if (!path.is_nonempty()) 
   {
    file_name->reset();
    return;
   }

  // Search backwards for a parameter, which is a normally unused field in a
  // URL delimited by a semicolon. We parse the parameter as part of the
  // path, but here, we don't want to count it. The last semicolon is the
  // parameter. The path should start with a slash, so we don't need to check
  // the first one.
  int file_end = path.end();
  for (int i = path.end() - 1; i > path.begin; i--) {
    if (spec[i] == ';') {
      file_end = i;
      break;
    }
  }

  // Now search backwards from the filename end to the previous slash
  // to find the beginning of the filename.
  for (int i = file_end - 1; i >= path.begin; i--) {
    if (IsURLSlash(spec[i])) {
      // File name is everything following this character to the end
      *file_name = MakeRange(i + 1, file_end);
      return;
    }
  }

  // No slash found, this means the input was degenerate (generally paths
  // will start with a slash). Let's call everything the file name.
  *file_name = MakeRange(path.begin, file_end);
  return;
}

bool DoExtractQueryKeyValue(const char* spec,Component* query,Component* key,Component* value) 
{
  if (!query->is_nonempty())
    return false;
  int start = query->begin;
  int cur = start;
  int end = query->end();

  // We assume the beginning of the input is the beginning of the "key" and we
  // skip to the end of it.
  key->begin = cur;
  while (cur < end && spec[cur] != '&' && spec[cur] != '=')
    cur++;
  key->len = cur - key->begin;

  // Skip the separator after the key (if any).
  if (cur < end && spec[cur] == '=')
    cur++;

  // Find the value part.
  value->begin = cur;
  while (cur < end && spec[cur] != '&')
    cur++;
  value->len = cur - value->begin;

  // Finally skip the next separator if any
  if (cur < end && spec[cur] == '&')
    cur++;

  // Save the new query
  *query = MakeRange(cur, end);
  return true;
}

Parsed::Parsed() {}
Parsed::~Parsed() {}

int Parsed::Length() const 
{
  if (ref.is_valid())
    return ref.end();
  return CountCharactersBefore(REF, false);
}

int Parsed::CountCharactersBefore(ComponentType type,bool include_delimiter) const 
{
  if (type == SCHEME)
    return scheme.begin;

  // There will be some characters after the scheme like "://" and we don't
  // know how many. Search forwards for the next thing until we find one.
  int cur = 0;
  if (scheme.is_valid())
    cur = scheme.end() + 1;  // Advance over the ':' at the end of the scheme.

  if (username.is_valid()) {
    if (type <= USERNAME)
      return username.begin;
    cur = username.end() + 1;  // Advance over the '@' or ':' at the end.
  }

  if (password.is_valid()) {
    if (type <= PASSWORD)
      return password.begin;
    cur = password.end() + 1;  // Advance over the '@' at the end.
  }

  if (host.is_valid()) {
    if (type <= HOST)
      return host.begin;
    cur = host.end();
  }

  if (port.is_valid()) {
    if (type < PORT || (type == PORT && include_delimiter))
      return port.begin - 1;  // Back over delimiter.
    if (type == PORT)
      return port.begin;  // Don't want delimiter counted.
    cur = port.end();
  }

  if (path.is_valid()) {
    if (type <= PATH)
      return path.begin;
    cur = path.end();
  }

  if (query.is_valid()) {
    if (type < QUERY || (type == QUERY && include_delimiter))
      return query.begin - 1;  // Back over delimiter.
    if (type == QUERY)
      return query.begin;  // Don't want delimiter counted.
    cur = query.end();
  }

  if (ref.is_valid()) {
    if (type == REF && !include_delimiter)
      return ref.begin;  // Back over delimiter.

    // When there is a ref and we get here, the component we wanted was before
    // this and not found, so we always know the beginning of the ref is right.
    return ref.begin - 1;  // Don't want delimiter counted.
  }

  return cur;
}

bool ExtractScheme(const char* url, int url_len, Component* scheme) {
    return DoExtractScheme(url, url_len, scheme);
}

void ExtractFileName(const char* url,const Component& path,Component* file_name) {
  DoExtractFileName(url, path, file_name);
}


bool ExtractQueryKeyValue(const char* url,Component* query,Component* key,Component* value) 
{
    return DoExtractQueryKeyValue(url, query, key, value);
}

void ParseAuthority(const char* spec,const Component& auth,Component* username,
                    Component* password,Component* hostname,Component* port_num) 
{
    DoParseAuthority(spec, auth, username, password, hostname, port_num);
}
int ParsePort(const char* url, const Component& port) 
{
    return DoParsePort(url, port);
}
void ParseStandardURL(const char* url, int url_len, Parsed* parsed) 
{
    DoParseStandardURL(url, url_len, parsed);
}

void ParseAfterScheme(const char* spec,int spec_len,int after_scheme,Parsed* parsed) 
{
    DoParseAfterScheme(spec, spec_len, after_scheme, parsed);
}

