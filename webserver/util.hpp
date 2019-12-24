#pragma once


#include<iostream>
#include<string>
#include<algorithm>
#include<vector>
#include<sstream>

class Util
{
    public:
        static void StringToLower(std::string &s)
        {
            std::transform(s.begin(),s.end(),s.begin(),::tolower);
        }
        static void StringToUpper(std::string &s)
        {
            std::transform(s.begin(),s.end(),s.begin(),::toupper);
        }
        static void TansfromToVector(std::string &s,std::vector<std::string> &v)
        {
            size_t start = 0;
            while(1)
            {
            size_t pos = s.find("\n",start);
            if(std::string::npos == pos)
            {
                break;
            }
            std::string sub_str = s.substr(start,pos-start);
            v.push_back(sub_str);
            start = pos +1;
            }
        }
        static void MakeKV(std::string s,std::string &key,std::string &value)
        {
            size_t pos = s.find(":",0);
            if(std::string::npos == pos)
            {
                return ;
            }
            key = s.substr(0,pos);
            value = s.substr(pos+2);
        }
        static int StringToInt(std::string s)
        {
            std::stringstream ss(s);
            int result = 0;
            ss>>result;
            return result;

        }
        static std::string IntToString(int i)
        {
            std::stringstream ss;
            ss<<i;
            return ss.str();
        }
        static std::string CodeTodec(int code)
        {
            switch(code)
            {
                case 200:
                    return "OK";
                case 404:
                    return "NOT FOUNT";
                default:
                    return "Unknow";
            }
        }
        static std::string Suffixtotype(const std::string &suffix)
        {
            std::string ct = "Content-Type: ";
            if(suffix == ".html")
            {
                ct += "text/html";
            }
            else if(suffix == ".css")
            {
                ct += "text/css";
            }
            else if(suffix == ".js")
            {
                ct += "application/x-javascript";
            }
            else if(suffix == ".jpg")
            {
                ct += "image/jpeg";
            }
            else if(suffix == ".png")
            {
                ct += "image/png";
            }
            else if(suffix == ".mp4")
            {
                ct +="video/mpeg4";
            }
            else
            {
                ct += "text/html";
            }
            return ct;
        }
};
