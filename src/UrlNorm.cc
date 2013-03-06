/**
* Copyright (c) 2011-2012 Pangoso
* @version 1.0.0
* @author Zhai Zhouwei
* @email zhaizhouwei@panguso.com
* @date 2012-5-21
* @description URL标准化实现
*/
#include <string.h>
#include "UrlNorm.h"
#include "UrlNormUtil.h"


/*----------------------------去掉URL中间的空白字符-----------------------------------*/
// 是否空白字符
inline bool IsRemovableURLWhitespace(int ch) {
  return ch == '\r' || ch == '\n' || ch == '\t';
}

// 去掉URL中间的空白字符
template<typename CHAR>
const CHAR* DoRemoveURLWhitespace(const CHAR* input, int input_len,CanonOutputT<CHAR>* buffer,int* output_len) {
  // Fast verification that there's nothing that needs removal. This is the 99%
  // case, so we want it to be fast and don't care about impacting the speed when we do find whitespace.
  int found_whitespace = false;
  for (int i = 0; i < input_len; i++) {
    if (!IsRemovableURLWhitespace(input[i]))
      continue;
    found_whitespace = true;
    break;
  }

  if (!found_whitespace) {
    // Didn't find any whitespace, we don't need to do anything. We can just return the input as the output.
    *output_len = input_len;
    return input;
  }

  // Remove the whitespace into the new buffer and return it.
  for (int i = 0; i < input_len; i++) {
    if (!IsRemovableURLWhitespace(input[i]))
      buffer->push_back(input[i]);
  }
  *output_len = buffer->length();
  return buffer->data();
}
const char* RemoveURLWhitespace(const char* input, int input_len,CanonOutputT<char>* buffer,int* output_len) {
  return DoRemoveURLWhitespace(input, input_len, buffer, output_len);
}
/*----------------------------------end of RemoveURLWhitespace-------------------------------------------*/


// Contains the canonical version of each possible input letter in the scheme
// (basically, lower-cased). The corresponding entry will be 0 if the letter is not allowed in a scheme.
const char kSchemeCanonical[0x80] = {
// 00-1f: all are invalid
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
//  ' '   !    "    #    $    %    &    '    (    )    *    +    ,    -    .    /
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  '+',  0,  '-', '.',  0,
//   0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ?
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
//   @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
     0 , 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
//   P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',  0,   0 ,  0,   0 ,  0,
//   `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o
     0 , 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
//   p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',  0 ,  0 ,  0 ,  0 ,  0 };

// This could be a table lookup as well by setting the high bit for each valid character, 
//but it's only called once per URL, and it makes the lookup table easier to read not having extra stuff in it.
inline bool IsSchemeFirstChar(unsigned char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/*-----------------------标准化Scheme---------------------*/
template<typename CHAR, typename UCHAR>
bool DoScheme(const CHAR* spec,const Component& scheme,CanonOutput* output,Component* out_scheme) {
  if (scheme.len <= 0) {
    // Scheme is unspecified or empty, convert to empty by appending a colon.
    *out_scheme = Component(output->length(), 0);
    output->push_back(':');
    return true;
  }
  // The output scheme starts from the current position.
  out_scheme->begin = output->length();
  bool success = true;
  int end = scheme.end();
  for (int i = scheme.begin; i < end; i++) {
    UCHAR ch = static_cast<UCHAR>(spec[i]);
    char replacement = 0;
    if (ch < 0x80) {
      if (i == scheme.begin) {
        // Need to do a special check for the first letter of the scheme.
        if (IsSchemeFirstChar(static_cast<unsigned char>(ch)))
          replacement = kSchemeCanonical[ch];
      } else {
        replacement = kSchemeCanonical[ch];
      }
    }

    if (replacement) {
      output->push_back(replacement);
    } else if (ch == '%') {
      success = false;
      output->push_back('%');
    } else {
      // Invalid character.
      success = false;
    }
  }

  // The output scheme ends with the the current position, before appending the colon.
  out_scheme->len = output->length() - out_scheme->begin;
  output->push_back(':');
  return success;
}

bool NormalizeScheme(const char* spec,const Component& scheme,CanonOutput* output,Component* out_scheme) {
  return DoScheme<char, unsigned char>(spec, scheme, output, out_scheme);
}
/*-----------------------------end of scheme-----------------------------*/

/*------------------------------标准化UserInfo------------------------------------*/
// The username and password components reference ranges in the corresponding
// *_spec strings. Typically, these specs will be the same (we're
// canonicalizing a single source string), but may be different when replacing components.
template<typename CHAR, typename UCHAR>
bool DoUserInfo(const CHAR* username_spec,
                const Component& username,
                const CHAR* password_spec,
                const Component& password,
                CanonOutput* output,
                Component* out_username,
                Component* out_password) {
  if (username.len <= 0 && password.len <= 0) {
    // Common case: no user info. We strip empty username/passwords.
    *out_username = Component();
    *out_password = Component();
    return true;
  }

  // Write the username.
  out_username->begin = output->length();
  if (username.len > 0) {
    // This will escape characters not valid for the username.
    AppendStringOfType(&username_spec[username.begin], username.len,
                       CHAR_USERINFO, output);
  }
  out_username->len = output->length() - out_username->begin;

  // When there is a password, we need the separator. Note that we strip
  // empty but specified passwords.
  if (password.len > 0) {
    output->push_back(':');
    out_password->begin = output->length();
    AppendStringOfType(&password_spec[password.begin], password.len,
                       CHAR_USERINFO, output);
    out_password->len = output->length() - out_password->begin;
  } else {
    *out_password = Component();
  }

  output->push_back('@');
  return true;
}
bool NormalizeUserInfo(const char* username_source,
                          const Component& username,
                          const char* password_source,
                          const Component& password,
                          CanonOutput* output,
                          Component* out_username,
                          Component* out_password) {
  return DoUserInfo<char, unsigned char>(username_source, username, password_source, password,output, out_username, out_password);
}
/*------------------------------end of UserInfo------------------------------------*/


/*--------------------------------标准化port---------------------------------------*/
// Helper functions for converting port integers to strings.
inline void WritePortInt(char* output, int output_len, int port) {
  _itoa_s(port, output, output_len, 10);
}

// This function will prepend the colon if there will be a port.
template<typename CHAR, typename UCHAR>
bool DoPort(const CHAR* spec,const Component& port,int default_port_for_scheme,CanonOutput* output,
     Component* out_port) {
  int port_num = ParsePort(spec, port);
  if (port_num == PORT_UNSPECIFIED ||
      port_num == default_port_for_scheme) {
    *out_port = Component();
    return true;  // Leave port empty.
  }

  if (port_num == PORT_INVALID) {
    // Invalid port: We'll copy the text from the input so the user can see
    // what the error was, and mark the URL as invalid by returning false.
    output->push_back(':');
    out_port->begin = output->length();
    AppendInvalidNarrowString(spec, port.begin, port.end(), output);
    out_port->len = output->length() - out_port->begin;
    return false;
  }

  // Convert port number back to an integer. Max port value is 5 digits, and
  // the Parsed::ExtractPort will have made sure the integer is in range.
  const int buf_size = 6;
  char buf[buf_size];
  WritePortInt(buf, buf_size, port_num);

  // Append the port number to the output, preceeded by a colon.
  output->push_back(':');
  out_port->begin = output->length();
  for (int i = 0; i < buf_size && buf[i]; i++)
    output->push_back(buf[i]);

  out_port->len = output->length() - out_port->begin;
  return true;
}
bool NormalizePort(const char* spec,
                      const Component& port,
                      int default_port_for_scheme,
                      CanonOutput* output,
                      Component* out_port) {
  return DoPort<char, unsigned char>(spec, port,default_port_for_scheme,output, out_port);
}
/*-----------------------------------end of port-----------------------------------------------*/


/*-----------------------------------标准化Ref----------------------------------------------*/
template<typename CHAR, typename UCHAR>
void DoNormalizeRef(const CHAR* spec,const Component& ref,CanonOutput* output,Component* out_ref) {
  if (ref.len < 0) {
    // Common case of no ref.
    *out_ref = Component();
    return;
  }

  // Append the ref separator. Note that we need to do this even when the ref is empty but present.
  output->push_back('#');
  out_ref->begin = output->length();

  // Now iterate through all the characters, converting to UTF-8 and validating.
  int end = ref.end();
  for (int i = ref.begin; i < end; i++) {
    if (spec[i] == 0) {
      // IE just strips NULLs, so we do too.
      continue;
    } else if (static_cast<UCHAR>(spec[i]) < 0x20) {
      AppendEscapedChar(static_cast<unsigned char>(spec[i]), output);
    } else if (static_cast<UCHAR>(spec[i]) < 0x80) {
      // Normal ASCII characters are just appended.
      output->push_back(static_cast<char>(spec[i]));
    } else {
      // Non-ASCII characters are appended unescaped
      unsigned code_point;
      ReadUTFChar(spec, &i, end, &code_point);
      AppendUTF8Value(code_point, output);
    }
  }

  out_ref->len = output->length() - out_ref->begin;
}
void NormalizeRef(const char* spec,const Component& ref,CanonOutput* output,Component* out_ref) {
  DoNormalizeRef<char, unsigned char>(spec, ref, output, out_ref);
}
/*------------------------------end of Ref------------------------------------------*/


/*----------------------------标准化URL主函数--------------------------------------*/
//template<typename CHAR, typename UCHAR>
bool DoNormalizeStandardURL(const URLComponentSource<char>& source,
                               const Parsed& parsed,
                               CharsetConverter* query_converter,
                               CanonOutput* output,
                               Parsed* new_parsed) {
  // Scheme: this will append the colon.
  bool success = NormalizeScheme(source.scheme, parsed.scheme,output, &new_parsed->scheme);

  // Authority (username, password, host, port)
  bool have_authority;
  if (parsed.username.is_valid() || parsed.password.is_valid() ||
      parsed.host.is_nonempty() || parsed.port.is_valid()) {
    have_authority = true;

    // Only write the authority separators when we have a scheme.
    if (parsed.scheme.is_valid()) {
      output->push_back('/');
      output->push_back('/');
    }

    // User info: the canonicalizer will handle the : and @.
    //success &= NormalizeUserInfo(source.username, parsed.username,source.password, parsed.password,
    //                                output,&new_parsed->username,&new_parsed->password);

    success &= NormalizeHost(source.host, parsed.host,output, &new_parsed->host);

    // Host must not be empty for standard URLs.
    if (!parsed.host.is_nonempty())
      success = false;

    // Port: the port canonicalizer will handle the colon.
    int default_port = DefaultPortForScheme(&output->data()[new_parsed->scheme.begin], new_parsed->scheme.len);
    success &= NormalizePort(source.port, parsed.port, default_port,output, &new_parsed->port);
  } else {
    // No authority, clear the components.
    have_authority = false;
    new_parsed->host.reset();
    new_parsed->username.reset();
    new_parsed->password.reset();
    new_parsed->port.reset();
    success = false;  // Standard URLs must have an authority.
  }

  // Path
  if (parsed.path.is_valid()) {
    success &= NormalizePath(source.path, parsed.path,output, &new_parsed->path);
  } else if (have_authority || parsed.query.is_valid() || parsed.ref.is_valid()) {
    // When we have an empty path, make up a path when we have an authority
    // or something following the path. The only time we allow an empty
    // output path is when there is nothing else.
    new_parsed->path = Component(output->length(), 1);
    output->push_back('/');
  } else {
    // No path at all
    new_parsed->path.reset();
  }

  // Query
  NormalizeQuery(source.query, parsed.query, query_converter,
                    output, &new_parsed->query);

  // Ref: ignore failure for this, since the page can probably still be loaded.
  //NormalizeRef(source.ref, parsed.ref, output, &new_parsed->ref);

  return success;
}

// Returns the default port for the given canonical scheme, or PORT_UNSPECIFIED
// if the scheme is unknown.
int DefaultPortForScheme(const char* scheme, int scheme_len) {
  int default_port = PORT_UNSPECIFIED;
  switch (scheme_len) {
    case 4:
      if (!strncmp(scheme, "http", scheme_len))
        default_port = 80;
      break;
    case 5:
      if (!strncmp(scheme, "https", scheme_len))
        default_port = 443;
      break;
    case 3:
      if (!strncmp(scheme, "ftp", scheme_len))
        default_port = 21;
      else if (!strncmp(scheme, "wss", scheme_len))
        default_port = 443;
      break;
    case 6:
      if (!strncmp(scheme, "gopher", scheme_len))
        default_port = 70;
      break;
    case 2:
      if (!strncmp(scheme, "ws", scheme_len))
        default_port = 80;
      break;
  }
  return default_port;
}

bool NormalizeStandardURL(const char* spec,
                             int spec_len,
                             const Parsed& parsed,
                             CharsetConverter* query_converter,
                             CanonOutput* output,
                             Parsed* new_parsed) {
 return DoNormalizeStandardURL(URLComponentSource<char>(spec), parsed, query_converter,output, new_parsed);
}
