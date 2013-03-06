/*

*/
#include "UrlDomain.h"

#include <string.h>

inline const char * RevMatch(const char * str, int len, const char *pattern)
{
	while(len > 0 && *pattern != 0)
	{
		len--;
		if(str[len] != *pattern)
		{
			break;
		}
		pattern++;
	}
	if(*pattern == 0)
	{
		return str + len;
	}
	else
	{
		return 0;
	}
}

inline const char * RevDot(const char * str, const char * dot)
{
	dot--;
	while(dot > str)
	{
		if(*dot == '.')
		{
			return dot + 1;
		}
		dot--;
	}
	return 0;
}

static const char* GetLastTwoParts(const char* url)
{
	if(url == 0)
	{
		return url;
	}
	int urllen = strlen(url);
	int dotcount = 0;
	for(int i=urllen - 1;i>=0;--i)
	{
		if(*(url + i) == '.')
		{
			dotcount++;
		}
		if(dotcount == 2)
		{
			return (url + i + 1);
		}
	}

	return url;
}

const char * GetDomainFromHost(const char * url)
{
	const char *result = NULL;
	if(url == 0)
		return 0;
	int urllen = strlen(url);
	if(urllen == 0)
	{
		return 0;
	}
	for(int i = 0; top_domain[i] != 0; i++)
	{
		const char * match;
		match = RevMatch(url, urllen, top_domain[i]);
		if(match != 0)
		{
			// 以顶级通用域名结尾，将倒数第二节开始的部分作为域名
			result = RevDot(url, match);
			// 如果不足两节，则将整个域名部分作为主域名
			if(result == 0)
			{
				return url;
			}
			else
			{
				return result;
			}
		}
	}
	for(int i = 0; domain_map[i].top_domain != 0; i++)
	{
		// 以特殊的几个国家域名结尾，包括cn,tw,hk,uk,jp,il,nz,kr
		const char * match;
		match = RevMatch(url, urllen, domain_map[i].top_domain);
		if(match != 0)
		{
			for(int s = 0; domain_map[i].second_domain[s] != 0; s++)
			{
				const char * match2;
				match2 = RevMatch(url, urllen, domain_map[i].second_domain[s]);
				if(match2 != 0)
				{
					// 位置在二级域名列表中，故以倒数第三节开始的部分作为域名
					result = RevDot(url, match2);
					if(result == 0)
					{
						// 如果出错，则返回整个url
						return url;
					}
					else
					{
						return result;
					}
				}
			}
			// 不在二级域名列表中，将后两节作为域名
			result = RevDot(url, match);
			// 如果不足两节，则将整个域名部分作为主域名
			if(result == 0)
			{
				return url;
			}
			else
			{
				return result;
			}
		}
	}
	for(int i = 0; top_country[i] != 0; i++)
	{
		// 以国家域名结尾
		const char * match;
		match = RevMatch(url, urllen, top_country[i]);
		if(match != 0)
		{
			for(int s = 0; general_2nd_domain[s] != 0; s++)
			{
				const char * match2;
				match2 = RevMatch(url, match - url, general_2nd_domain[s]);
				if(match2 != 0)
				{
					// 倒数第二节位于general_2nd_domain中，将后三节作为域名
					result = RevDot(url, match2);
					if(result == 0)
					{
						// 如果出错，则返回整个url
						return url;
					}
					else
					{
						return result;
					}
				}
			}
			// 不在二级域名列表中，将后两节作为域名
			result = RevDot(url, match);
			// 如果不足两节，则将整个域名部分作为主域名
			if(result == 0)
			{
				return url;
			}
			else
			{
				return result;
			}
		}
	}
	// 既不以通用域名结尾，也不以国家域名结尾，取后两节作为域名，若不足两节，则以整个串作为域名
	return GetLastTwoParts(url);
}

/*
**test
int main()
{
  const char * url = "www.panguso.com";
  const char * re_url = GetDomainFromHost(url);
  printf("domain:%s\n",re_url);
  return 0;
}
*/
