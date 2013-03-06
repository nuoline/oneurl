/*
*/ 

#include "UrlNorm.h"
#include "UrlNormUtil.h"
//#include <iostream>
//
// Our query canonicalization
// --------------------------
// We escape all non-ASCII characters and control characters, like Firefox.
// This is more conformant to the URL spec, and there do not seem to be many
// problems relating to Firefox's behavior.
// Returns true if the characters starting at |begin| and going until |end|
// (non-inclusive) are all representable in 7-bits.
template<typename CHAR, typename UCHAR>
bool IsAllASCII(const CHAR* spec, const Component& query) {
  int end = query.end();
  for (int i = query.begin; i < end; i++) {
    if (static_cast<UCHAR>(spec[i]) >= 0x80)
      return false;
  }
  return true;
}

// Appends the given string to the output, escaping characters that do not
// match the given |type| in SharedCharTypes. This version will accept 8 or 16
// bit characters, but assumes that they have only 7-bit values. It also assumes
// that all UTF-8 values are correct, so doesn't bother checking
template<typename CHAR>
void AppendRaw8BitQueryString(const CHAR* source, int length,
                              CanonOutput* output) {
  for (int i = 0; i < length; i++) {
    if (!IsQueryChar(static_cast<unsigned char>(source[i])))
      AppendEscapedChar(static_cast<unsigned char>(source[i]), output);
    else  // Doesn't need escaping.
      output->push_back(static_cast<char>(source[i]));
  }
}

// Runs the converter on the given UTF-8 input. Since the converter expects
// UTF-16, we have to convert first. The converter must be non-NULL.
void RunConverter(const char* spec,
                  const Component& query,
                  CharsetConverter* converter,
                  CanonOutput* output) {
  // This function will replace any misencoded values with the invalid
  // character. This is what we want so we don't have to check for error.
  RawCanonOutputW<1024> utf16;
  ConvertUTF8ToUTF16(&spec[query.begin], query.len, &utf16);
  converter->ConvertFromUTF16(utf16.data(), utf16.length(), output);
}

template<typename CHAR, typename UCHAR>
void DoConvertToQueryEncoding(const CHAR* spec,
                              const Component& query,
                              CharsetConverter* converter,
                              CanonOutput* output) {
  if (IsAllASCII<CHAR, UCHAR>(spec, query)) {
    // Easy: the input can just appended with no character set conversions.
    AppendRaw8BitQueryString(&spec[query.begin], query.len, output);

  } else {
    // Harder: convert to the proper encoding first.
    if (converter) {
      // Run the converter to get an 8-bit string, then append it, escaping
      // necessary values.
      RawCanonOutput<1024> eight_bit;
      RunConverter(spec, query, converter, &eight_bit);
      AppendRaw8BitQueryString(eight_bit.data(), eight_bit.length(), output);

    } else {
      // No converter, do our own UTF-8 conversion.
      AppendStringOfType(&spec[query.begin], query.len, CHAR_QUERY, output);
    }
  }
}

template<typename CHAR, typename UCHAR>
void DoNormalizeQuery(const CHAR* spec,
                         const Component& query,
                         CharsetConverter* converter,
                         CanonOutput* output,
                         Component* out_query) {
  if (query.len < 0) {
    *out_query = Component();
    return;
  }

  output->push_back('?');
  out_query->begin = output->length();

  DoConvertToQueryEncoding<CHAR, UCHAR>(spec, query, converter, output);

  out_query->len = output->length() - out_query->begin;
}


void NormalizeQuery(const char* spec,
                       const Component& query,
                       CharsetConverter* converter,
                       CanonOutput* output,
                       Component* out_query) {
  DoNormalizeQuery<char, unsigned char>(spec, query, converter,
                                           output, out_query);
}
