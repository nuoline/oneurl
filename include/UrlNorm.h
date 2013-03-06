/*
* Copyright (c) 2011-2012 Pangoso
* @version 1.0.0
* @author Zhai Zhouwei
* @email zhaizhouwei@panguso.com
* @date 2012-5-20
* @description URL归一化数据结构以及函数头文件
* @references Chrome/gurl
*/
#ifndef PANGU_URL_NORM_H__
#define PANGU_URL_NORM_H__

#include <memory.h>
#include <stdlib.h>

#include "UrlParse.h"
#include "string16.h"


//定义URL标准化输出的结构体
template<typename T>
class CanonOutputT {
  public:
  CanonOutputT() : buffer_(NULL), buffer_len_(0), cur_len_(0) {}
  virtual ~CanonOutputT() {}

  // 调整buffer的大小
  virtual void Resize(int sz) = 0;

  // 根据位置返回字符
  inline char at(int offset) const {
    return buffer_[offset];
  }

  //根据位置设置字符
  inline void set(int offset, int ch) {
    buffer_[offset] = ch;
  }

  // 返回buffer当前长度.
  inline int length() const {
    return cur_len_;
  }

  //返回buffer容量
  int capacity() const {
    return buffer_len_;
  }

  // 调用这个函数可输出buffer
  const T* data() const {
    return buffer_;
  }
  T* data() {
    return buffer_;
  }

  // 设置当前buffer长度
  void set_length(int new_len) {
    cur_len_ = new_len;
  }
  
  //得到栈顶元素
 const T get_pop() {
   if (cur_len_ < 1)
     return ' ';//栈空则返回space字符
   return buffer_[cur_len_ - 1];
 }
 
  //出栈函数 add 2012-06-13 nuoline
  void pop_back() {
    if (cur_len_ < 1) { //empty
      return;
    }
    cur_len_--;
  }
  
  //进栈函数 
  void push_back(T ch) {
    if (cur_len_ < buffer_len_) {
      buffer_[cur_len_] = ch;
      cur_len_++;
      return;
    }

    // Grow容量
    if (!Grow(1))
      return;
    buffer_[cur_len_] = ch;
    cur_len_++;
  }

  // 追加字符串到输出output.
  void Append(const T* str, int str_len) {
    if (cur_len_ + str_len > buffer_len_) {
      if (!Grow(cur_len_ + str_len - buffer_len_))
        return;
    }
    for (int i = 0; i < str_len; i++)
      buffer_[cur_len_ + i] = str[i];
    cur_len_ += str_len;
  }

 protected:
  // 扩张buffer
  bool Grow(int min_additional) {
    static const int kMinBufferLen = 16;
    int new_len = (buffer_len_ == 0) ? kMinBufferLen : buffer_len_;
    do {
      if (new_len >= (1 << 30))  // Prevent overflow below.
        return false;
      new_len *= 2;
    } while (new_len < buffer_len_ + min_additional);
    Resize(new_len);
    return true;
  }

  T* buffer_;
  int buffer_len_;

  // Used characters in the buffer.
  int cur_len_;
};
// Simple implementation of the CanonOutput using new[]. This class
// also supports a static buffer so if it is allocated on the stack, most
// URLs can be canonicalized with no heap allocations.
template<typename T, int fixed_capacity = 1024>
class RawCanonOutputT : public CanonOutputT<T> {
 public:
  RawCanonOutputT() : CanonOutputT<T>() {
    this->buffer_ = fixed_buffer_;
    this->buffer_len_ = fixed_capacity;
  }
  virtual ~RawCanonOutputT() {
    if (this->buffer_ != fixed_buffer_)
      delete[] this->buffer_;
  }

  virtual void Resize(int sz) {
    T* new_buf = new T[sz];
    memcpy(new_buf, this->buffer_,
           sizeof(T) * (this->cur_len_ < sz ? this->cur_len_ : sz));
    if (this->buffer_ != fixed_buffer_)
      delete[] this->buffer_;
    this->buffer_ = new_buf;
    this->buffer_len_ = sz;
  }

 protected:
  T fixed_buffer_[fixed_capacity];
};

typedef CanonOutputT<char> CanonOutput;
typedef CanonOutputT<char16> CanonOutputW;

template<int fixed_capacity>
class RawCanonOutput : public RawCanonOutputT<char, fixed_capacity> {};
template<int fixed_capacity>
class RawCanonOutputW : public RawCanonOutputT<char16, fixed_capacity> {};

class CharsetConverter {
 public:
  CharsetConverter() {}
  virtual ~CharsetConverter() {}

  // Converts the given input string from UTF-16 to whatever output format the
  // converter supports. This is used only for the query encoding conversion,
  // which does not fail. Instead, the converter should insert "invalid
  // character" characters in the output for invalid sequences, and do the
  // best it can.
  //
  // If the input contains a character not representable in the output
  // character set, the converter should append the HTML entity sequence in
  // decimal, (such as "&#20320;") with escaping of the ampersand, number
  // sign, and semicolon (in the previous example it would be
  // "%26%2320320%3B"). This rule is based on what IE does in this situation.
  virtual void ConvertFromUTF16(const char16* input,int input_len,CanonOutput* output) = 0;
};


//去掉URL中间的空白字符
const char* RemoveURLWhitespace(const char* input, int input_len,CanonOutputT<char>* buffer,int* output_len);

// IDN ------------------------------------------------------------------------

// Converts the Unicode input representing a hostname to ASCII using IDN rules.
// The output must fall in the ASCII range, but will be encoded in UTF-16.
//
// On success, the output will be filled with the ASCII host name and it will
// return true. Unlike most other canonicalization functions, this assumes that
// the output is empty. The beginning of the host will be at offset 0, and
// the length of the output will be set to the length of the new host name.
//
// On error, returns false. The output in this case is undefined.
bool IDNToASCII(const char16* src, int src_len, CanonOutputW* output);

// Piece-by-piece canonicalizers ----------------------------------------------
//标准化scheme
bool NormalizeScheme(const char* spec,const Component& scheme,CanonOutput* output,Component* out_scheme);

//标准化Userinfo
bool NormalizeUserinfo(const char* username_source,
                       const Component& username,
                       const char* password_source,
                       const Component& password,
                       CanonOutput* output,
                       Component* out_username,
                       Component* out_password);

//记录 IP/Host 标准化时的详细状态结构体
struct CanonHostInfo {
  CanonHostInfo() : family(NEUTRAL), num_ipv4_components(0), out_host() {}

  // Convenience function to test if family is an IP address.
  bool IsIPAddress() const { return family == IPV4 || family == IPV6; }

  // This field summarizes how the input was classified by the canonicalizer.
  enum Family {
    NEUTRAL,   // - Doesn't resemble an IP address.  As far as the IP
               //   canonicalizer is concerned, it should be treated as a
               //   hostname.
    BROKEN,    // - Almost an IP, but was not canonicalized.  This could be an
               //   IPv4 address where truncation occurred, or something
               //   containing the special characters :[] which did not parse
               //   as an IPv6 address.  Never attempt to connect to this
               //   address, because it might actually succeed!
    IPV4,      // - Successfully canonicalized as an IPv4 address.
    IPV6,      // - Successfully canonicalized as an IPv6 address.
  };
  Family family;

  // If |family| is IPV4, then this is the number of nonempty dot-separated
  // components in the input text, from 1 to 4.  If |family| is not IPV4,
  // this value is undefined.
  int num_ipv4_components;

  // Location of host within the canonicalized output.
  // NormalizeIPAddress() only sets this field if |family| is IPV4 or IPV6.
  // NormalizeHostVerbose() always sets it.
  Component out_host;

  // |address| contains the parsed IP Address (if any) in its first
  // AddressLength() bytes, in network order. If IsIPAddress() is false
  // AddressLength() will return zero and the content of |address| is undefined.
  unsigned char address[16];

  // Convenience function to calculate the length of an IP address corresponding
  // to the current IP version in |family|, if any. For use with |address|.
  int AddressLength() const {
    return family == IPV4 ? 4 : (family == IPV6 ? 16 : 0);
  }
};

//标准化host
// The 8-bit version requires UTF-8 encoding.  Use this version when you only
// need to know whether canonicalization succeeded.
bool NormalizeHost(const char* spec,const Component& host,CanonOutput* output,Component* out_host);

// 标准化IP addresses.
void NormalizeIPAddress(const char* spec,const Component& host,CanonOutput* output,CanonHostInfo* host_info);

//标准化port 
bool NormalizePort(const char* spec,const Component& port,int default_port_for_scheme,CanonOutput* output,Component* out_port);

//得到协议的默认端口号
int DefaultPortForScheme(const char* scheme, int scheme_len);

//标准化Path
bool NormalizePath(const char* spec,const Component& path,CanonOutput* output,Component* out_path);

// 标准化Query: Prepends the ? if needed.
// The converter can be NULL. In this case, the output encoding will be UTF-8.
void NormalizeQuery(const char* spec,const Component& query,CharsetConverter* converter,CanonOutput* output,
                                Component* out_query);

//标准化 Ref: 
//Prepends the # if needed. The output will be UTF-8 (this is the only canonicalizer that does not produce ASCII output).
// The output is guaranteed to be valid UTF-8.
void NormalizeRef(const char* spec,const Component& path,CanonOutput* output,Component* out_path);

//标准化StandardURL
bool NormalizeStandardURL(const char* spec,
                          int spec_len,
                          const Parsed& parsed,
                          CharsetConverter* query_converter,
                          CanonOutput* output,
                          Parsed* new_parsed);

// Part replacer --------------------------------------------------------------

// 用于存储URL子部分时的内部数据结构
template<typename CHAR>
struct URLComponentSource {
  // Constructor normally used by callers wishing to replace components. This
  // will make them all NULL, which is no replacement. The caller would then
  // override the components they want to replace.
  URLComponentSource()
      : scheme(NULL),
        username(NULL),
        password(NULL),
        host(NULL),
        port(NULL),
        path(NULL),
        query(NULL),
        ref(NULL) {
  }

  // Constructor normally used internally to initialize all the components to
  // point to the same spec.
  explicit URLComponentSource(const CHAR* default_value)
      : scheme(default_value),
        username(default_value),
        password(default_value),
        host(default_value),
        port(default_value),
        path(default_value),
        query(default_value),
        ref(default_value) {
  }

  const CHAR* scheme;
  const CHAR* username;
  const CHAR* password;
  const CHAR* host;
  const CHAR* port;
  const CHAR* path;
  const CHAR* query;
  const CHAR* ref;
};
#endif
