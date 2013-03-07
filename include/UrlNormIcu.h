/**
* @version 1.0.0
* @email zhaizhouwei@nuoline.com
* @date 2012-5-20
* @description ICU integration functions.
* @references Chrome/gurl
*/

#ifndef PANGU_URL_NORM_ICU_H_
#define PANGU_URL_NORM_ICU_H_

#include "UrlNorm.h"

typedef struct UConverter UConverter;

//gbk(gb18030) to utf-8
int Gbk2Utf8(char *target,
             int32_t targetCapacity,
             const char *source,
             int32_t sourceLength);

// An implementation of CharsetConverter that implementations can use to
// interface the canonicalizer with ICU's conversion routines.
class ICUCharsetConverter : public CharsetConverter {
 public:
  // Constructs a converter using an already-existing ICU character set
  // converter. This converter is NOT owned by this object; the lifetime must
  // be managed by the creator such that it is alive as long as this is.
  ICUCharsetConverter(UConverter* converter);

  virtual ~ICUCharsetConverter();

  virtual void ConvertFromUTF16(const char16* input,int input_len,CanonOutput* output);

 private:
  // The ICU converter, not owned by this class.
  UConverter* converter_;
};

#endif
