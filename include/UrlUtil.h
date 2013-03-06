/*
* Copyright (c) 2011-2012 Pangoso
* @version 1.0.0
* @author Zhai Zhouwei
* @email zhaizhouwei@panguso.com
* @date 2012-5-24
* @description URL归一化函数封装头文件i
* @references Chrome/gurl
*/
#ifndef _PANGU_URL_UTIL_H_
#define _PANGU_URL_UTIL_H_
#include <string>

#include "UrlParse.h"
#include "UrlNorm.h"

/*---------Init---------*/
void Initialize();

void Shutdown();

//比较scheme
bool FindAndCompareScheme(const char* str,int str_len,const char* compare,Component* found_scheme);
inline bool FindAndCompareScheme(const std::string& str,const char* compare,Component* found_scheme) {
  return FindAndCompareScheme(str.data(), static_cast<int>(str.size()),compare, found_scheme);
}
// Given a string and a range inside the string, compares it to the given
// lower-case |compare_to| buffer.
bool CompareSchemeComponent(const char* spec,const Component& component,const char* compare_to);

// 是否是standard URL. This means that
bool IsStandard(const char* spec,const Component& scheme);

/*--------URL library封装----------*/
//the charset converter can  be NULL to use UTF-8 (it will be faster in this case)
bool Normalize(const char* spec,
                        int spec_len,
                        CharsetConverter* charset_converter,
                        CanonOutput* output,
                        Parsed* output_parsed);

/*---------------String相关函数------------------*/
/*定义字母大写转化小写表*/
const char to_lower_t[0x80] = {
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

char char2lower(char c);

//to lower
std::string to_lower(const char *pszValue);

/*定义字母小写转化为大写表*/
const char to_upper_t[0x80] = {
// 00-1f: all are invalid
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
//  ' '   !    "    #    $    %    &    '    (    )    *    +    ,    -    .    /
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  '+',  0,  '-', '.',  0,
//   0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ?
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
//   @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
     0 , 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
//   P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',  0,   0 ,  0,   0 ,  0,
//   `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o
     0 , 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
//   p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',  0 ,  0 ,  0 ,  0 ,  0 };

/** 把字符转为大写 */
char char2upper(char c);

/** 把字符串转为大写 */
std::string to_upper(const char *pszValue);
// Compare the lower-case form of the given string against the given ASCII
// string.  This is useful for doing checking if an input string matches some
// token, and it is optimized to avoid intermediate string copies.
bool LowerCaseEqualsASCII(const char* a_begin,const char* a_end,const char* b);

bool LowerCaseEqualsASCII(const char* a_begin,const char* a_end,const char* b_begin,const char* b_end);

// Unescapes the given string using URL escaping rules.
void DecodeURLEscapeSequences(const char* input, int length,CanonOutputW* output);

// Escapes the given string as defined by the JS method encodeURIComponent.  See
// https://developer.mozilla.org/en/JavaScript/Reference/Global_Objects/encodeURIComponent
void EncodeURIComponent(const char* input, int length,CanonOutput* output);

//
class StdStringCanonOutput : public CanonOutput {
 public:
  StdStringCanonOutput(std::string* str)
      : CanonOutput(),
        str_(str) {
    cur_len_ = static_cast<int>(str_->size());  // Append to existing data.
    str_->resize(str_->capacity());
    buffer_ = str_->empty() ? NULL : &(*str_)[0];
    buffer_len_ = static_cast<int>(str_->size());
  }
  virtual ~StdStringCanonOutput() {
    // Nothing to do, we don't own the string.
  }

  // Must be called after writing has completed but before the string is used.
  void Complete() {
    str_->resize(cur_len_);
    buffer_len_ = cur_len_;
  }

  virtual void Resize(int sz) {
    str_->resize(sz);
    buffer_ = str_->empty() ? NULL : &(*str_)[0];
    buffer_len_ = sz;
  }

 protected:
  std::string* str_;
};

#endif
