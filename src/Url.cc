/**
* Copyright (c) 2011-2012
* @version 1.0.0
* @author Zhai Zhouwei
* @email nuoline@gmail.com
* @date 2012-5-29
* @description URL归一化对外接口实现
*/
#include <vector>
#include <map>
#include <set>
#include <iostream>

#include "Url.h"
#include "UrlUtil.h"
#include "UrlDomain.h"
#include "UrlNormIcu.h"

using namespace std;

//构造函数
oneurl::oneurl() {     
}

//析构函数
oneurl::~oneurl() {
}

string strdefault[]={"index.html","index.htm","index.xml","index.shtml","index.xhtml","index.asp","index.aspx","indxe.jsp","index.php","indexpage.html","indexpage.htm","indexpage.xml","indexpage.shtml","indexpage.xhtml","indexpage.asp","indexpage.aspx","indxe.jsp","indexpage.php","home.html","home.htm","home.xml","home.shtml","home.xhtml","home.asp","home.aspx","indxe.jsp","home.php","homepage.html","homepage.htm","homepage.xml","homepage.shtml","homepage.xhtml","homepage.asp","homepage.aspx","indxe.jsp","homepage.php","default.html","default.htm","default.xml","default.shtml","default.xhtml","default.asp","default.aspx","indxe.jsp","default.php"};

const char HexChLookup[0x10] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};

//Split
void Split(const string str_value,const string str_spliter,vector<string>& v_str_list)
{
        v_str_list.clear();
        if (str_value.empty() || str_spliter.empty())
                return;
        string str_value_ = str_value;
        size_t nPos = str_value_.find(str_spliter);
        string strTemp;
        while (string::npos != nPos)
        {
            strTemp = str_value_.substr(0,nPos);
            if (!strTemp.empty() && strTemp.compare("=")!=0)
                v_str_list.push_back(strTemp);
            str_value_ = str_value_.substr(nPos+str_spliter.size());
            nPos = str_value_.find(str_spliter);
        }
        if(!str_value_.empty())
          v_str_list.push_back(str_value_);
}

string PreProcess(const char* url,int url_len)
{
  std::string strurl = url;
  int begin = 0;
  while (begin < url_len && ShouldTrimFromURL(url[begin]))
    begin++;
  strurl = strurl.substr(begin,url_len-begin);
  if (begin == url_len)
    return "";  // Input is empty or all whitespace.
  for (int i = begin; i < url_len; i++) {
    if (url[i] == ':')
      return strurl;
   }
  std::string default_sc("http://");
  return default_sc.append(strurl);
}

//remove the superfluous '/'
void RemoveSurplusSlash(const char* path,char* pc_path,int len)
{
   bool flag=true;
   int i,j=0;
   for(i=0;i<len;i++)
   {
     if(path[i]!=47)
       {
          pc_path[j++] = path[i];
          flag=true;
       }
     else if(flag)
       {
          pc_path[j++] = path[i];
          flag=false;
       }
    else
      continue;
   }
   pc_path[j]=0;
}

//extract the filename form path
string ExtractFileName(string path)
{
   string filename;
   size_t found;
   found = path.rfind("/");
   if(found != string::npos)
     filename = path.substr(found+1);
   else filename = "";
   return filename;
}

bool FreFileName(const string path)
{
   string stmp = path.substr(0,path.length()-1);
   size_t found;
   string filename;
   found = stmp.rfind("/");
   if(found != string::npos)
     filename = path.substr(found+1);
   if(filename.find(".") != string::npos)
    return true;
   else return false;
}

//pct encoding of gbk
void PrePctofGBK(const char* src, int srclen,char* pcturl)
{
   int i,j=0;
   for(i=0;i<srclen;i++)
   {
     if(src[i] < 0) {
       pcturl[j++] = '%';
       pcturl[j++] = HexChLookup[(src[i] >> 4) & 0xf];
       pcturl[j++] = HexChLookup[src[i] & 0xf];
       i++;
       pcturl[j++] = '%';
       pcturl[j++] = HexChLookup[(src[i] >> 4) & 0xf];
       pcturl[j++] = HexChLookup[src[i] & 0xf];
     }
     else
     {
        pcturl[j++] = src[i];
     }
   }
   pcturl[j] = 0; 
}

//precc url %
void ProccPct(const char* src, int srclen, char* dest)
{
   int i,j=0;
   for(i=0;i<srclen;i++) {
      if(src[i] == '%') {
         dest[j++] = src[i++];
         dest[j++] = char2upper(src[i++]);
         dest[j++] = char2upper(src[i]);
      }
      else
         dest[j++] = char2lower(src[i]);
   }
   dest[j] = 0;
}

//
bool oneurl::ParseUrl(const string str_in)
{
  RawCanonOutputT<char> whitespace_buffer;
  int spec_len;
  const char* spec = RemoveURLWhitespace(str_in.c_str(), str_in.length(), &whitespace_buffer, &spec_len);
  spec_ = spec;
  Component scheme;
  if (!ExtractScheme(spec, spec_len, &scheme))
    return false;
  if (IsStandard(spec, scheme))
     // All "normal" URLs.
     ParseStandardURL(spec, spec_len, &parsed_);
  else 
     return false;
  return true;
}

bool oneurl::Parse(const string &str_in)
{
  if(str_in.length() > 1024*5)
     return false;
   spec_.clear();
   StdStringCanonOutput output(&spec_);
   int in_len = str_in.length();
   bool ok = Normalize(str_in.c_str(),in_len,NULL,&output,&parsed_);
   return ok;
}

bool AllEmpty(std::map<string,string> &map_query)
{
  std::map<string,string> ::iterator itt;
  for (itt = map_query.begin(); itt != map_query.end(); itt++)
      if(!(itt->second.empty()))
          return false;
  return true;
}

string oneurl::CNormalize(const string str_in, const int codetype)
{
   if(str_in.length() > 1024*5)
     return string();
   //load the defalut filename to set
   set<string> setFilename;
   for (int i=0; i<45; i++) setFilename.insert(strdefault[i]); 
   
   spec_.clear();
   StdStringCanonOutput output(&spec_);
   string str_inurl;
   //= PreProcess(str_in.c_str(),str_in.length());
   
   //procc the pct and to_upper
   char pc_inurl[1024*5];
   ProccPct(str_in.c_str(),str_in.length(),pc_inurl);   
   str_inurl =pc_inurl;
   //if codetype is 1,procc gbk encoding (for chinese)
   if(codetype == 1) {
   char target[1024*5];
   PrePctofGBK(str_inurl.c_str(), str_inurl.length(), target);
   str_inurl = target;
   }

   int in_len = str_inurl.length();
   bool ok = Normalize(str_inurl.c_str(),in_len,NULL,&output,&parsed_);
   if (!ok)
      return string();
   output.Complete();
   string normurl;//the output url;

   //host, the "www"
   //host,add the "www"
   Component host = parsed_.host;
   string strhost = ComponentString(host);
   while(strhost.compare(0,8,"www.www.") == 0)
     strhost = strhost.substr(4);

   //port
   Component port = parsed_.port;
   string strport = ComponentString(port);  
  
   //path remove the surplus slash
   Component path = parsed_.path;
   string str_path = ComponentString(path);
   //str_path = ProccPath(str_path.c_str(), str_path.length());
   char cpath[path.len];
   if(!str_path.empty()){
     RemoveSurplusSlash(str_path.c_str(), cpath, str_path.length());//remove the surplus slash here
    }
    string strpath = cpath;
    string strfname = ExtractFileName(strpath);
    if(!strfname.empty()) {
     if(setFilename.find(strfname) != setFilename.end())
       strpath = strpath.substr(0,strpath.length()-strfname.length());//remove the default filename
     else if((strfname.find('.') == string::npos) && (parsed_.query.len <= 0))
      strpath.append("/");
    }
    else {
      if(FreFileName(strpath) && (parsed_.query.len <= 0))
        strpath = strpath.substr(0,strpath.length()-1);
        strfname = ExtractFileName(strpath);
        if(setFilename.find(strfname) != setFilename.end())
          strpath = strpath.substr(0,strpath.length()-strfname.length());
    }
   
    //sort query start
   Component comp = parsed_.query;
   string str_query_("");//sorted query
   size_t nPos;
   string key,value;
   int kcount = 0;
   if(comp.len>0)
    {
      string str_query = string(spec_, comp.begin, comp.len);
      vector<string> vstr_list;
      Split(str_query,"&",vstr_list);
      if(vstr_list.size()==1)
         {
            nPos = str_query.find("=");
            if(nPos!=string::npos) {
               key = str_query.substr(0,nPos);
               value = str_query.substr(nPos+1);
            }
            else {
               key = str_query;
               value = "";
            }
            key = to_lower(key.c_str());//key to lower
            if(!value.empty() && value.compare("=")!=0)
              str_query_ = key + "=" + value;//query中只有一个参数不用排序
            else
              str_query_ = key;//spec_.substr(0,spec_.length()-1);//空value，去掉"="
         }
      else {
       std::map<string,string> map_query;
       std::map<string,string>::iterator mit;
       vector<string>::iterator it;
       string str_each;
       //vector<string> v_e_query; 
       for(it=vstr_list.begin();it<vstr_list.end();it++)
          {
              str_each = *it;
              nPos = str_each.find("=");
              if(nPos!=string::npos)
                  key = str_each.substr(0,nPos);
              else 
                 { key = str_each;
                   value = "";
		   map_query[key] = value;//add
                   continue;}
              value = str_each.substr(nPos+1);//"value"
              mit = map_query.find(key);
              if(mit == map_query.end()) {
               //map_query.insert(pair<string,string>(key,value));
               map_query[key] = value;
              }
              else {
               if((*mit).second.empty())
                   map_query[key] = value; 
                }
           }
        std::map<string,string> ::reverse_iterator ritt;
	std::map<string,string> ::iterator itt;
        if(AllEmpty(map_query))
          {
            ritt = map_query.rbegin();
            if(ritt != map_query.rend())
               str_query_ = ritt->first;
          }
        else {
          for (itt = map_query.begin(); itt != map_query.end(); itt++){
            if (!(itt->second.empty()))
            str_query_ += itt->first +"="+ itt->second + "&";//sorted_query
          }
          str_query_ = str_query_.substr(0,str_query_.length()-1);
        }
       }
    }//sort query end
   
   //normurl = spec_.substr(0,parsed_.host.end());
   normurl = spec_.substr(0,host.begin);
   if(host.len > 0)
     normurl.append(strhost);
   if(!strport.empty()) {
     normurl.append(":");
     normurl.append(strport);
     }
   if(strpath.length() > 0)
     normurl.append(strpath);
   if(!str_query_.empty()) {
     normurl.append("?");
     normurl.append(str_query_);
     }

  return normurl;
}

//是否动态页面
bool oneurl::IsDynamic() {
   Component query,path;
   query = parsed_.query;
   path = parsed_.path;
   if (query.len < 0) 
      return false;
   size_t found1,found2;
   found1 = spec_.find(";",path.begin,path.len);
   found2 = spec_.find("=",path.begin,path.len);
   if (found1!=string::npos || found2!=string::npos)
     return true;
}

//获取域名
string oneurl::GetDomain() const {
   Component host = parsed_.host;
   if (host.len < 0)
      return string();
   if(IsIp)
      return ComponentString(host);
   string str_host = ComponentString(host);
   if(str_host.empty())
      return string();
   else
     {
      const string str_domain = GetDomainFromHost(str_host.c_str());
      return str_domain;
     }
}

//获取路径深度
int oneurl::PathDepth() {
  Component path = parsed_.path;
  if (path.len < 0)
    return 0;
  int count = 0;
  string str_path = ComponentString(path);
  for (size_t i = 0; i < str_path.size(); ++i)
	{
		if ('/' == str_path[i]) 
		{
                  ++count;
		}
                else continue;
	}
  return count;
}

//函数归一化接口
string Normalize(const char* spec,int spec_len,int codetype) {
   if(spec_len > 1024*5)
     return string();
   //load the defalut filename to set
   set<string> setFilename;
   for (int i=0; i<45; i++) setFilename.insert(strdefault[i]);
   
   char pc_strin[1024*5];
   ProccPct(spec,spec_len,pc_strin);
   string strin = pc_strin;
   //if codetype is 1,procc gbk encoding (for chinese)
   if(codetype == 1) {
   char target[1024*5];
   PrePctofGBK(strin.c_str(), strin.length(), target);
   strin = target;
   }

   string str_out;
   StdStringCanonOutput output(&str_out);
   Parsed output_parsed;
   bool ok = Normalize(strin.c_str(),strin.length(),NULL,&output,&output_parsed);
   if (!ok)
      return string();
   output.Complete();
   string normurl;//the output url;
   
   //host,the "www"
   Component host = output_parsed.host;
   string strhost = string(str_out,host.begin,host.len);
   while(strhost.compare(0,8,"www.www.") == 0)
     strhost = strhost.substr(4);    

   //port
   Component port = output_parsed.port;
   string strport;
   if(port.len > 0)
     strport  = string(str_out,port.begin,port.len);  

   //path remove the surplus slash
   Component path = output_parsed.path;
   string str_path;
   if(path.len > 0)
     str_path = string(str_out,path.begin,path.len);
   char cpath[path.len];
   if(!str_path.empty()){
     RemoveSurplusSlash(str_path.c_str(), cpath, str_path.length());//remove the surplus slash here
    }
   string strpath = cpath;
    string strfname = ExtractFileName(strpath);
    if(!strfname.empty()) {
     if(setFilename.find(strfname) != setFilename.end())
       strpath = strpath.substr(0,strpath.length()-strfname.length());//remove the default filename
     else if((strfname.find('.') == string::npos) && (output_parsed.query.len <= 0))
      strpath.append("/");
    }
    else {
      if(FreFileName(strpath) && (output_parsed.query.len <= 0))
        strpath = strpath.substr(0,strpath.length()-1);
        strfname = ExtractFileName(strpath);
        if(setFilename.find(strfname) != setFilename.end())
          strpath = strpath.substr(0,strpath.length()-strfname.length());
    }
  
   //query 
   Component comp = output_parsed.query;
   string str_query_("");//sorted query
   size_t nPos;
   string key,value;
   if(comp.len>0)
    {
      string str_query = string(str_out, comp.begin, comp.len);
      vector<string> vstr_list;
      Split(str_query,"&",vstr_list);
      if(vstr_list.size()==1)
         {
            nPos = str_query.find("=");
            if(nPos!=string::npos) {
               key = str_query.substr(0,nPos);
               value = str_query.substr(nPos+1);
            }
            else {
               key = str_query;
               value = "";
            }
            key = to_lower(key.c_str());//key to lower
            if(!value.empty() && value.compare("=")!=0)
              str_query_ = key + "=" + value;//query中只有一个参数不用排序
            else
              str_query_ = key;//spec_.substr(0,spec_.length()-1);//空value，去掉"="
         }
      else {
       std::map<string,string> map_query;
       std::map<string,string>::iterator mit;
       vector<string>::iterator it;
       string str_each;
       //vector<string> v_e_query;   
       for(it=vstr_list.begin();it<vstr_list.end();it++)
          {
              str_each = *it;
              nPos = str_each.find("=");
              if(nPos!=string::npos)
                  key = str_each.substr(0,nPos);
              else
                 { key = str_each;
                   value = "";
		   map_query[key] = value;//add
                   continue;}
              value = str_each.substr(nPos+1);//"value"
              mit = map_query.find(key);
              if(mit == map_query.end()) {
               //map_query.insert(pair<string,string>(key,value));
               map_query[key] = value;
              }
              else {
               if((*mit).second.empty())
                   map_query[key] = value;
                }
           }
	std::map<string,string> ::reverse_iterator ritt;
        std::map<string,string> ::iterator itt;
        if(AllEmpty(map_query))
          {
            ritt = map_query.rbegin();
            if(ritt != map_query.rend())
               str_query_ = ritt->first;
          }
        else 
        {
          for (itt = map_query.begin(); itt != map_query.end(); itt++){
            if (!(itt->second.empty()))
            str_query_ += itt->first +"="+ itt->second + "&";//sorted_query
          }
          str_query_ = str_query_.substr(0,str_query_.length()-1);
        }
       }
    }//sort query end
  
    normurl = str_out.substr(0,host.begin);
   if(host.len > 0)
     normurl.append(strhost);
   if(!strport.empty()) {
     normurl.append(":");
     normurl.append(strport);
     }
   if(strpath.length() > 0)
     normurl.append(strpath);
   if(!str_query_.empty()) {
     normurl.append("?");
     normurl.append(str_query_);
     }

  return normurl; 
}
/*
int main()
{
    const char *url = "http://search.panguso.com/pagesearch.htm?q=%E4%B8%AD%E5%9B%BD&u=nuoline&u=nuoline&a=test";
    int len = strlen(url);
    string str_test = Normalize(url,len);
    cout<<str_test<<endl; 
}
*/
