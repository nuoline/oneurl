/*
* Copyright (c) 2011-2012 Pangoso
* @version 1.0.0
* @author Zhai Zhouwei
* @email zhaizhouwei@panguso.com
* @date 2012-5-20
* @description URL的IP归一化头文件
* @references Chrome/gurl
*/

#ifndef PANGU_URL_NORM_IP_H_
#define PANGU_URL_NORM_IP_H_

#include "UrlNorm.h"
#include "UrlParse.h"

bool FindIPv4Components(const char* spec,const Component& host,Component components[4]);

// Converts an IPv4 address to a 32-bit number (network byte order).
// Possible return values:
//   IPV4    - IPv4 address was successfully parsed.
//   BROKEN  - Input was formatted like an IPv4 address, but overflow occurred
//             during parsing.
//   NEUTRAL - Input couldn't possibly be interpreted as an IPv4 address.
//             It might be an IPv6 address, or a hostname.
// On success, |num_ipv4_components| will be populated with the number of
// components in the IPv4 address.
CanonHostInfo::Family IPv4AddressToNumber(
    const char* spec,
    const Component& host,
    unsigned char address[4],
    int* num_ipv4_components);

// Converts an IPv6 address to a 128-bit number (network byte order), returning
// true on success. False means that the input was not a valid IPv6 address.
bool IPv6AddressToNumber(const char* spec,const Component& host,unsigned char address[16]);
#endif
