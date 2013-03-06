/**
* Copyright (c) 2011-2012
* @version 1.0.0
* @author Zhai Zhouwei
* @email nuoline@gmail.com
* @date 2012-5-29
* @description URL归一化测试
*/
#include <iostream>
#include <string>
#include <string.h>

#include "Url.h"
using namespace std;

struct UrlCase {
    const char* input;
    const char* output;
    const char* expected;
  } cases[] = {
    //scheme
    //{"        www.panguso.cOM/", "","http://www.panguso.com/"},
    //{"			www.PANGUSO.cn", "","http://www.panguso.com"},
    //{"http:////////www.panguso.com", "","http://www.panguso.com"},
    //host
    //{"http://WWW.pANGUSO.cOM","","http://www.panguso.com"},
    //{"http://%62%61%6f%62%61%6f%31%37%38.com/","","http://baobao178.com/"},
    //ip
    //{"http://192.0x00A80001", "","192.168.0.1"},
    //{"0.0.0xFFFF","","0.0.255.255"},
     // Backslashes
    //{"http:\\\\www.panguso.com\\nuoline", ""},
    //path
    //{"http://www/nuoline%2Ehtml", ""},
    //{"http://www.panguso.com/././nuoline","",""},
    //{"http://www.panguso.com/nuoline/bar/../","",""},
    //{"http://%62%61%6f%62%61%6f%31%37%38.com/","",""},
    //{"http://www.panguso.com/nuoline/%2e#ddDd","",""},
    //query
    //{"http:////search.panguso.com/pagesearch.htm?q=%E4%B8%AD%E5%9B%BD&u=nuoline&u=nuoline&a=test#ddddddd","",""},
    //{"http://www.ifeng.com/tech/zhuanti/qbsss/zxbd/news?aid=24271932&amp;mid=9k6YRf","",""},
    //{"http://www.baidu.com/index.php?a=中国","",""},
    {"http://www.hongxingstone.com/products_detail/&productId=3ffac778-de27-4bf4-a0be-767d5b2c9281&comp_stats=comp-FrontProducts_list01-1292210400607.html","",""},
    //{"http://www.lhd2013.com/GeneralContentShow.html?GeneralContentShow_DocI%E0v&%06%00%00%00%002470ec68778f6ba5a68f145dcd","",""},
    {"http://www.www.baidu.com","",""},
    {"http:////%D6%D0%D0%C4%B6%AF%CC%AC.asp ","",""},
};

int main()
{
    
   for (size_t i = 0; i < sizeof(cases)/sizeof(UrlCase); i++) {
      const string url = cases[i].input;
      //const char* url = cases[i].input;
      oneurl curl;
      //curl.Parse(url);
      string str_test = curl.CNormalize(url);
      //string str_test = Normalize(url.c_str(),url.length());
      cout<<url<<" -> "<<str_test<<endl;
      //cout<<curl.GetHost()<<endl;
      //cout<<url<<endl;
      //cout<<"scheme:"<<curl.GetScheme()<<endl;
      //cout<<"user:"<<curl.GetUsername()<<endl;
      //cout<<"pw:"<<curl.GetPassword()<<endl;
      //cout<<"port:"<<curl.GetPort()<<endl;
      //cout<<"domain->"<<curl.GetDomain()
      cout<<"host->"<<curl.GetHost()<<endl;
      cout<<"domain->"<<curl.GetDomain()<<endl;
      //cout<<"path:"<<curl.GetPath()<<endl;
      //cout<<"query:"<<curl.GetQuery()<<endl;
      //cout<<"Depth:"<<curl.PathDepth()<<endl;
    }
    return 0; 
}
