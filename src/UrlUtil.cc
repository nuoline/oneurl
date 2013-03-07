/**
* Copyright (c) 2011-2012
* @version 1.0.0
* @author Zhai Zhouwei
* @email nuoline@gmail.com
* @date 2012-5-20
* @description URL归一化封装实现
* @references Chrome/gurl
*/

#include <stdio.h>
#include <vector>

#include "UrlUtil.h"
#include "UrlNormUtil.h"

/*----------------------char and string------------------------*/
// ASCII-specific tolower
template <class Char> inline Char ToLowerASCII(Char c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// Backend for LowerCaseEqualsASCII.
template<typename Iter>
inline bool DoLowerCaseEqualsASCII(Iter a_begin, Iter a_end, const char* b) {
  for (Iter it = a_begin; it != a_end; ++it, ++b) {
    if (!*b || ToLowerASCII(*it) != *b)
      return false;
  }
  return *b == 0;
}

bool LowerCaseEqualsASCII(const char* a_begin,const char* a_end,const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

bool LowerCaseEqualsASCII(const char* a_begin,const char* a_end,
                          const char* b_begin,const char* b_end) {
  while (a_begin != a_end && b_begin != b_end &&
         ToLowerASCII(*a_begin) == *b_begin) {
    a_begin++;
    b_begin++;
  }
  return a_begin == a_end && b_begin == b_end;
}


char char2lower(char c)
{
        if (c >= 'A' && c <= 'Z')
                 return to_lower_t[c];
        else
                return c;
}
std::string to_lower(const char *pszValue)
{
    std::string  strRet = pszValue;
    size_t dwLen = strRet.size();
    for (unsigned long i = 0 ; i < dwLen; i ++)
    {
                if (strRet[i] < 0)
                {
                        i++;
                }
                else
                {
                        strRet[i] = char2lower(pszValue[i]);
                }
    }
    return strRet;
}

char char2upper(char c)
{
        if (c >= 'a' && c <= 'z')
                return to_upper_t[c];
        else
                return c;
}

std::string to_upper(const char *pszValue)
{
    std::string  strRet = pszValue;
    size_t dwLen = strRet.size();
    for (unsigned long i = 0 ; i < dwLen; i ++)
    {
                if (pszValue[i] < 0)
                {
                        i++;
                }
                else
                {
                        strRet[i] = char2upper(pszValue[i]);
                }
    }
    return strRet;
}

/*---------------------end----------------------*/

/*---------------------Init begin-----------------------------*/
const int kNumStandardURLSchemes = 6;
const char* kStandardURLSchemes[kNumStandardURLSchemes] = {
  "http",
  "https",
  "ftp",
  "gopher",
  "ws",  // WebSocket.
  "wss",  // WebSocket secure.
};

std::vector<const char*>* standard_schemes = NULL;

// Ensures that the standard_schemes list is initialized, does nothing if it already has values.
void InitStandardSchemes() {
  if (standard_schemes)
    return;
  standard_schemes = new std::vector<const char*>;
  for (int i = 0; i < kNumStandardURLSchemes; i++)
    standard_schemes->push_back(kStandardURLSchemes[i]);
}

//------------Init-----------------
void Initialize() {
  InitStandardSchemes();
}

//-----------Cleanup--------------
void Shutdown() {
  if (standard_schemes) {
    delete standard_schemes;
    standard_schemes = NULL;
  }
}
/*----------------------Init end-----------------------------------*/

/*-----------------------Compare Scheme-----------------------------*/
// Given a string and a range inside the string, compares it to the given lower-case |compare_to| buffer.
template<typename CHAR>
inline bool DoCompareSchemeComponent(const CHAR* spec,const Component& component,const char* compare_to) {
  if (!component.is_nonempty())
    return compare_to[0] == 0;  // When component is empty, match empty scheme.
  return LowerCaseEqualsASCII(&spec[component.begin],&spec[component.end()],compare_to);
}
bool CompareSchemeComponent(const char* spec,
                            const Component& component,
                            const char* compare_to) {
  return DoCompareSchemeComponent(spec, component, compare_to);
}

template<typename CHAR>
bool DoFindAndCompareScheme(const CHAR* str,
                            int str_len,
                            const char* compare,
                            Component* found_scheme) {
  // Before extracting scheme, canonicalize the URL to remove any whitespace.
  // This matches the canonicalization done in DoNormalize function.
  RawCanonOutputT<CHAR> whitespace_buffer;
  int spec_len;
  const CHAR* spec = RemoveURLWhitespace(str, str_len,&whitespace_buffer, &spec_len);

  Component our_scheme;
  if (!ExtractScheme(spec, spec_len, &our_scheme)) {
    // No scheme.
    if (found_scheme)
      *found_scheme = Component();
    return false;
  }
  if (found_scheme)
    *found_scheme = our_scheme;
  return DoCompareSchemeComponent(spec, our_scheme, compare);
}

bool FindAndCompareScheme(const char* str,
                          int str_len,
                          const char* compare,
                          Component* found_scheme) {
  return DoFindAndCompareScheme(str, str_len, compare, found_scheme);
}
/*------------------------end of Compare Scheme----------------------*/

/*--------------------------判断是否Standard URL----------------------*/
// Returns true if the given scheme identified by |scheme| within |spec| is one of the registered "standard" schemes.
bool DoIsStandard(const char* spec, const Component& scheme) {
  if (!scheme.is_nonempty())
    return false;  // Empty or invalid schemes are non-standard.

  InitStandardSchemes();
  for (size_t i = 0; i < standard_schemes->size(); i++) {
    if (LowerCaseEqualsASCII(&spec[scheme.begin], &spec[scheme.end()],standard_schemes->at(i)))
      return true;
  }
  return false;
}
//是否Standard URL
bool IsStandard(const char* spec, const Component& scheme) {
  return DoIsStandard(spec, scheme);
}
/*-------------------------end of 判断是否Standard URL-------------------*/


/*---------------------------URL标准化--------------------------------*/
template<typename CHAR>
bool DoNormalize(const CHAR* in_spec, int in_spec_len,
                    CharsetConverter* charset_converter,
                    CanonOutput* output,
                    Parsed* output_parsed) {
  // Remove any whitespace from the middle of the relative URL, possibly
  // copying to the new buffer.
  RawCanonOutputT<CHAR> whitespace_buffer;
  int spec_len;
  const CHAR* spec = RemoveURLWhitespace(in_spec, in_spec_len,&whitespace_buffer, &spec_len);

  Parsed parsed_input;
  Component scheme;
  if (!ExtractScheme(spec, spec_len, &scheme))
    return false;

  // This is the parsed version of the input URL, we have to canonicalize it before storing it in our object.
  bool success;
  if (DoIsStandard(spec, scheme)) {
    // All "normal" URLs.
    ParseStandardURL(spec, spec_len, &parsed_input);
    success = NormalizeStandardURL(spec, spec_len, parsed_input,charset_converter,output, output_parsed);

  } 
  else {
    // 其他URL类型不处理
    return false;
  }
  return success;
}

//URL标准化调用入口函数
bool Normalize(const char* spec,
                  int spec_len,
                  CharsetConverter* charset_converter,
                  CanonOutput* output,
                  Parsed* output_parsed) {
  return DoNormalize(spec, spec_len, charset_converter,output, output_parsed);
}

/*---------------------URL标准化 end----------------------------*/


/*----------------------Encode and Escape---------------*/
void DecodeURLEscapeSequences(const char* input, int length,
                              CanonOutputW* output) {
  RawCanonOutputT<char> unescaped_chars;
  for (int i = 0; i < length; i++) {
    if (input[i] == '%') {
      unsigned char ch;
      if (DecodeEscaped(input, &i, length, &ch)) {
        unescaped_chars.push_back(ch);
      } else {
        // Invalid escape sequence, copy the percent literal.
        unescaped_chars.push_back('%');
      }
    } else {
      // Regular non-escaped 8-bit character.
      unescaped_chars.push_back(input[i]);
    }
  }

  // Convert that 8-bit to UTF-16.
  for (int i = 0; i < unescaped_chars.length(); i++) {
    unsigned char uch = static_cast<unsigned char>(unescaped_chars.at(i));
    if (uch < 0x80) {
      // Non-UTF-8, just append directly
      output->push_back(uch);
    } else {
      // next_ch will point to the last character of the decoded
      // character.
      int next_character = i;
      unsigned code_point;
      if (ReadUTFChar(unescaped_chars.data(), &next_character,
                                 unescaped_chars.length(), &code_point)) {
        // Valid UTF-8 character, convert to UTF-16.
        AppendUTF16Value(code_point, output);
        i = next_character;
      } else {
        // If there are any sequences that are not valid UTF-8, we keep
        // invalid code points and promote to UTF-16. We copy all characters
        // from the current position to the end of the identified sequence.
        while (i < next_character) {
          output->push_back(static_cast<unsigned char>(unescaped_chars.at(i)));
          i++;
        }
        output->push_back(static_cast<unsigned char>(unescaped_chars.at(i)));
      }
    }
  }
}

void EncodeURIComponent(const char* input, int length,CanonOutput* output) {
  for (int i = 0; i < length; ++i) {
    unsigned char c = static_cast<unsigned char>(input[i]);
    if (IsComponentChar(c))
      output->push_back(c);
    else
      AppendEscapedChar(c, output);
  }
}
/*-----------------------end of Encode and Escape--------------------------*/
