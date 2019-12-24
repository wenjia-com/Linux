#pragma once


#include<iostream>
#include<vector>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string>
#include<fcntl.h>
#include<sstream>
#include<sys/sendfile.h>
#include<unordered_map>
#include"util.hpp"

using namespace std;

#define WWW "./wwwroot"
class httpRequest
{
    private:
        string request_line;
        string request_header;
        string request_body;
        string request_blank;
    private:
        string method;
        string uri;
        string version;
        unordered_map<string,string>header_kv;

        string path;//资源路径
        string query_string;//GET商船的参数
        bool cgi;
        int recource_size;
        string suffix;
    public:
        httpRequest():request_blank("\n"),path(WWW),recource_size(0)
        {
        }
        int GetRecource_size()
        {
            return  recource_size;
        }
        string &GetRequestHeader()
        {
            return request_header;
        }
        string &GetRequestLine()
        {
            return request_line;
        }
        string &GetRequestBody()
        {
            return request_body;
        }
        string GetPath()
        {
            return path;
        }
        bool MethodIsLegal()
        {
            //get/post
            if(method != "GET"&&method != "POST")
            {
                return false;
            }
            return true;
        }
        string GetSuffix()
        {
            return suffix;
        }
        void RequestLineParse()
        {
            stringstream ss(request_line);
            ss>>method>>uri>>version;

            Util::StringToUpper(method);

            cout<<"method: "<<method<<endl;
            cout<<"uri: "<<uri<<endl;
            cout<<"version: "<<version<<endl;

        }
        void RequestHeaderParse()
        {
            vector<string> v;
            Util::TansfromToVector(request_header,v);

            auto it = v.begin();
            for(;it!=v.end();it++)
            {
                string k;
                string v;
                Util::MakeKV(*it,k,v);
                header_kv.insert(make_pair(k,v));
                cout<<"key :"<<k<<endl;
                cout<<"value :"<<v<<endl;

            }
        }
        void UriParse()
        {
            if(method == "POST")
            {
                cgi = true;
                path += uri;
            }
            else
            {
                //GET
                size_t s = uri.find('?');
                if(string::npos == s)
                {
                    path += uri;
                }
                else
                {
                    cgi = true;
                    path += uri.substr(0,s);
                    query_string = uri.substr(s+1);

                }
            }
            if(path[path.size()-1] == '/')
            {
                path += "index.html";
            }
            size_t pos = path.rfind(".");
            if(string::npos == pos)
            {
                suffix = ".html";
            }
            else
            {
                suffix = path.substr(pos);
            }
            cout<<"debug: suffix ："<<suffix<<endl;
        }
        bool IsPathLegal()
        {
            struct stat st;
            if(stat(path.c_str(),&st) == 0)
            {
                if(S_ISDIR(st.st_mode))
                {
                    path += "/index.html";
                }
                else
                {
                    if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
                    {
                        cgi = true;
                    }
                }
                stat(path.c_str(),&st);
                recource_size = st.st_size;
                cout<<"recource_size "<<recource_size<<endl;
                return true;
            }
            else
            {
                return false;
            }
        }
        bool IsNeedRecv()
        {
            return method == "POST";
        }
        int GetContentLength()
        {
            auto it = header_kv.find("Content-Length");
            if(it == header_kv.end())
            {
                return -1;
            }
                return Util::StringToInt(it->second);
        }
        bool Iscgi()
        {
                return cgi;
        }
        ~httpRequest()
        {
        }
};

class httpResponse
{
    private:
        string response_line;//状态行
        string response_header;//响应报头
        string response_boty;//内容
        string response_blank;//空行
    private:
        int fd;
        int size;
    public:
        httpResponse():response_blank("\r\n"),fd(-1)
        {
        }
        void MakeResponseLine(int code)
        {
            response_line = "HTTP/1.1";
            response_line += " ";
            response_line += Util::IntToString(code);
            response_line += " ";
            response_line += Util::CodeTodec(code);
            response_line += "\r\n";
        }
        void MakeResponseHeader(vector<string>&v)
        {
            auto it = v.begin();
            for(;it != v.end();it++)
            {
                response_header += *it;
                response_header += "\r\n";
            }
        }
        void MakeResponse(httpRequest *rq,int code)
        {
            MakeResponseLine(code);
            string suffix = rq->GetSuffix();
            vector<string> v;
            size=rq->GetRecource_size();
            string ct = Util::Suffixtotype(suffix);
            //test
            cout<<"Content_Type: "<<ct<<endl;
            v.push_back(ct);
            string cl = "Content-Length: ";
            cl += Util::IntToString(size);
            v.push_back(cl);





            MakeResponseHeader(v);
            string path =rq->GetPath();
            
            cout<<"debug: "<<path.c_str()<<endl;
            fd = open(path.c_str(),O_RDONLY);
            cout<<" open fd = "<<fd<<endl;

        }
        string &GetResponseline()
        {
            return response_line;
        }
        string &GetResponseHeader()
        {
            return response_header;
        }
        string &GetResponseBlank()
        {
            return response_blank;
        }
        int GetFd()
        {
            return fd;
        }
        int GetRecourceSize()
        {
            return size;
        }
        ~httpResponse()
        {
            if(fd != -1)
            {
                close(fd);
            }
        }
};

class EndPoint
{
    private:
        int sock;
    public:
        EndPoint(int sock_):sock(sock_)
        {
        }
        int RecvLine(string &line)
        {
            char c = 'X';
            while(c != '\n')
            {
                ssize_t s = recv(sock,&c,1,0);
                if(s>0)
                {
                    if(c == '\r')
                    {//\n \r \r\n
                        if(recv(sock,&c,1,MSG_PEEK)>0)//窥探并不读取
                        {
                            if(c == '\n')
                            {
                            recv(sock,&c,1,0);//直接把窥探到的\n读取到c
                            }
                            else
                            {
                                //\r
                                c = '\n';
                            }

                        }
                        else
                        {
                            c = '\n';
                        }
                    }
                    line.push_back(c);
                }
                else
                {
                    c = '\n';
                    line.push_back(c);
                }
            }
            return line.size();
        }
        void RecvRequestLine(httpRequest *rq)
        {
            RecvLine(rq->GetRequestLine());
        }
        void RecvRequestHeader(httpRequest *rq)
        {
            string &rh = rq->GetRequestHeader();
            do{
            string line = "";
            RecvLine(line);
            if(line == "\n")
            {
                break;
            }
            rh += line;
            }while(1);
        }
        void RecvRequestBody(httpRequest *rq)
        {
            int len = rq->GetContentLength();
            cout<<"len:"<<len<<endl;
            string &body = rq->GetRequestBody();
            char c;
            while(len--)
            {
                if(recv(sock,&c,1,0)>0)
                {
                    body.push_back(c);
                }
            }

            cout<<"body :"<<body<<endl;
        }
        void SendResponse(httpResponse *rsp , bool cgi)
        {
            if(cgi == true)
            {
            }
            else
            {
                //GET无参
                string &response_line = rsp->GetResponseline();
                string response_header = rsp->GetResponseHeader();
                string response_blank = rsp->GetResponseBlank();
                int fd = rsp->GetFd();
                int size = rsp->GetRecourceSize();
                send(sock,response_line.c_str(),response_line.size(),0);
                send(sock,response_header.c_str(),response_header.size(),0);
                send(sock,response_blank.c_str(),response_blank.size(),0);
                sendfile(sock,fd,NULL,size);
            }
        }
        ~EndPoint()
        {
            if(sock>-1)
            {
              close(sock);
            }
        }
};
class Entry
{
    public:
        static void *HandlerRequest(void *args)
        {
            int code = 200;
            int *p = (int *)args;
            int sock = *p;
            EndPoint * ep = new EndPoint(sock);
            httpRequest *rq = new httpRequest();
            httpResponse *rsp = new httpResponse();
            //1.通过rndpoint,读取请求，并构建request
            //分析request，的出具体细节
            //通过endpoint返回response，最终完成请求/响应,close((sock));
            //尚书步骤可能彼此交叉进行
            //
            ep->RecvRequestLine(rq);
            rq->RequestLineParse();
            if(!rq->MethodIsLegal())
            {
                //feifafangfa
                code = 404;
                goto end;
            }
            ep->RecvRequestHeader(rq);
            rq->RequestHeaderParse();
            if(rq->IsNeedRecv())
            {
                ep->RecvRequestBody(rq);
            }
            if(!rq->IsPathLegal())
            {
                code = 404;
                goto end;
            }
            //method path query_string body version cgi
            if(rq->Iscgi() == true)//POST 或者GET带参
            {

            }
            else
            {
                rsp->MakeResponse(rq,code);
                ep->SendResponse(rsp,false);
            }

end:

            delete rq;
            delete rsp;
            delete ep;
            delete p;
        }
};
