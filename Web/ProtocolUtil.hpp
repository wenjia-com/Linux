#ifndef __PROTOCOLUTIL_HPP__
#define __PROTOCOLUTIL_HPP__

#include<iostream>
#include<stdlib.h>
#include<string>
#include<algorithm>
#include<unordered_map>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<strings.h>
#include<sstream>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<vector>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/sendfile.h>
using namespace std;

#define NORMAL 0
#define WARNING 1
#define ERROR 2

#define WEBROOT "wwwroot"
#define HOMEPAGE "index.html"
//错误级别数组
const char* ErrLevel[]={
    "Normal",
    "Warning",
    "Error"
};

void log(string msg,int level,string file,int line)
{
    cout<<"[ "<<file<<" ]"<<" : "<<line<<" : "<<msg<<"[ "<<ErrLevel[level]<<" ]"<<endl;
}

#define LOG(msg,level) log(msg,level,__FILE__,__LINE__);

class Util
{
    public:
        static void MakeKV(string s,string &k,string &v)
        {
            size_t pos=s.find(": ");
            k=s.substr(0,pos);
            v=s.substr(pos+2);
        }
        static string IntToString(int &x)
        {
            stringstream ss;
            ss<<x;
            return ss.str();
        }
        //返回code对应的状态描述
        static string CodeToDesc(int code)
        {
            switch(code)
            {
                case 404:
                    return "Not Found";
                case 200:
                    return "OK";
                default:
                    break;
            }
            return "Unkown";
        }
        static string SuffixToContent(string &suffix)
        {
            if(suffix==".css")
            {
                return "text/css";
            }
            if(suffix==".js")
            {
                return "application/x-javascript";
            }
            if(suffix==".html" || suffix==".htm")
            {
                return "text/html";
            }
            if(suffix==".jpg")
            {
                return "application/x-jpg";
            }
            return "text/html";
        }
};
class SocketApi
{
    public:
       static int Socket()
        {
            int sock=socket(AF_INET,SOCK_STREAM,0);
            if(sock < 0)
            {
                LOG("sock create error!",ERROR);
                exit(2);
            }
            int opt=1;
            setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
            return sock;
        }
        static void Bind(int sock,int port)
        {
            struct sockaddr_in local;
            //清空local
            bzero(&local,sizeof(local));
            local.sin_family=AF_INET;
            //动态绑定ip地址
            local.sin_addr.s_addr=htonl(INADDR_ANY);
            local.sin_port=htons(port);
            if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
            {
                LOG("bind error!",ERROR);
                exit(3);
            }
        }
        static void Listen(int sock)
        {
            const int backlog=5;
            if(listen(sock,backlog)<0)
            {
                LOG("listen error!",ERROR);
                exit(4);
            }
        }
        static int Accept(int listen_sock,string &ip,int &port)
        {
            struct sockaddr_in peer;    //填写客户端的sockaddr_in
            socklen_t  len=sizeof(peer);
            int sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
            if(sock<0)
            {
                LOG("accept error!",WARNING);
                exit(4);
            }
            port=ntohs(peer.sin_port);
            ip=inet_ntoa(peer.sin_addr);    //这里不用写成peer.sin_addr.s_addr的原因是这里是传参，不是赋值。传进去的结构体初始化为一个匿名的实体
            return sock;
        }
};

class Http_Response
{
    public:
        //协议响应字段
        string status_line;    //状态行
        vector<string> response_header;
        string blank;
        string response_text;
    private:
        int code;
        string path;    //响应的资源
        int recource_size;    //要响应的资源的大小

    public:
        Http_Response():blank("\r\n"),code(200),recource_size(0)
        {}
        int &Code()
        {
            return code;
        }
        void SetPath(string &path_)
        {
            path=path_;
        }
        void SetRecourceSize(int rs)
        {
            recource_size=rs;
        }
        string &Path()
        {
            return path;
        }
        int &RecourceSize()
        {
            return recource_size;
        }
        void MakeStatusLine()
        {
            status_line="HTTP/1.0";
            status_line+=" ";
            status_line+=Util::IntToString(code);
            status_line+=" ";
            status_line+=Util::CodeToDesc(code);
            status_line+="\r\n";
            LOG("Make StatusLine Done!",NORMAL);
        }
        void MakeResponseHeader()
        {
            string line;
            string suffix;    //提取路径里资源的后缀
            //Make Content-Type
            line="Content-Type: ";
            size_t pos=path.rfind('.');
            if(string::npos!=pos)
            {
                suffix=path.substr(pos);
                transform(suffix.begin(),suffix.end(),suffix.begin(),::tolower);
            }
            line+=Util::SuffixToContent(suffix);
            line+="\r\n";
            response_header.push_back(line);

            //Make Content-Length
            line="Content-Length: ";
            line+=Util::IntToString(recource_size);
            line+="\r\n";
            response_header.push_back(line);
            //空行
            line="\r\n";
            response_header.push_back(line);
            LOG("Make Response Header Done!",NORMAL);
        }
        ~Http_Response()
        {}
};

class Http_Request
{
    public:
        //基本协议字段
        string request_line;
        vector<string> request_header;
        string request_blank;
        string request_text;
    private:
        //解析字段
        string method;
        string uri;    //path?arg
        string version;    //版本
        string path;
        string query_string;    //参数
        unordered_map<string,string> header_kv;
        bool cgi;    //有参的话就要进行处理

    public:
        Http_Request():path(WEBROOT),cgi(false),request_blank("\r\n")
        {}
        //请求行解析
        void RequestLineParse()
        {
            //GET index.html HTTP/1.1
            stringstream ss(request_line);
            ss>>method>>uri>>version;
            //将method统一转为大写
            transform(method.begin(),method.end(),method.begin(),::toupper);
        }
        //我只应用两种请求方法
        bool IsMethodLegal()
        {
            if(method!="GET" && method!="POST")
            {
                LOG("Method is not Legal!",ERROR);
                return false;
            }
            return true;
        }
        void UriParse()
        {
            if(method=="GET")
            {
                size_t pos=uri.find('?');
                //有参
                if(pos!=string::npos)
                {
                    cgi=true;
                    path+=uri.substr(0,pos);
                    query_string=uri.substr(pos+1);
                }
                else
                {//无参
                    path+=uri;    //path="wwwroot/a/b/c.html"
                }
            }
            else   //POST
            {
                cgi=true;
                path+=uri;
            }    //wwwroot/

            if(path[path.size()-1]=='/')
            {
                path+=HOMEPAGE;    //wwwroot/index.html
            }
        }
        void HeaderParse()
        {
            string k,v;
            for(auto it=request_header.begin();it!=request_header.end();++it)
            {
                Util::MakeKV(*it,k,v);
                header_kv.insert({k,v});
            }
        }
        bool IsNeedRecv()
        {
            return (method=="POST") ? true : false;
        }

        int IsPathLegal(Http_Response *rsp)    //想要给Http_Response中的path获得值，以后要在sendText()中用到
        {
            int rs=0;   //资源大小
            struct stat st;
            if(stat(path.c_str(),&st)<0)
            {
                return 404;
            }
            else
            {
                rs=st.st_size;
                //判断文件是否是一个目录
                if(S_ISDIR(st.st_mode))
                {
                    path+="/";
                    path+=HOMEPAGE;
                    stat(path.c_str(),&st);
                    rs=st.st_size;
                }
                else if((st.st_mode & S_IXUSR)||\
                        (st.st_mode & S_IXGRP)||\
                        (st.st_mode & S_IXOTH))
                {//可执行文件
                    cgi=true;
                }
                else
                {
                    //
                }
            }
            rsp->SetPath(path);
            rsp->SetRecourceSize(rs);
            LOG("path is ok!",NORMAL);
            return 0;
        }
        bool IsCgi()
        {
            return cgi;
        }
        int ContentLength()
        {
            int content_length=-1;
            string cl=header_kv["Content-Length"];
            stringstream ss(cl);
            ss>>content_length;
            return content_length;
        }
        string GetParam()
        {
            if(method=="GET")
            {
                return query_string;
            }
            else
            {
                return request_text;
            }
        }
        ~Http_Request()
        {}
};

//读取请求包头数据
class Connect
{
    private:
        int sock;
    public:
        Connect(int sock_):sock(sock_)
        {}
        int RecvOneLine(string &line_)
        {
          char buff[1024];
          char c='X';
          int i=0;
          while(c!='\n'&&i<(1024-1))
          {
              ssize_t s=recv(sock,&c,1,0);
              //将/r /n /r/n ->/n
              if(s>0)
              {
                  if(c=='\r')
                  {
                      //探测下一个字符是不是'\n'
                      if(recv(sock,&c,1,MSG_PEEK)>0)
                      {
                          if(c=='\n')
                          {
                              recv(sock,&c,1,0);
                          }
                          else
                          {
                              //将'\r'赋值为'\n'
                              c='\n';
                          }
                      }
                  }
                  buff[i++]=c;
              }
              else
              {
                  break;
              }
          }
          buff[i]=0;    //给结尾加\0
          line_=buff;
          return i;     //返回读取到的长度
        }

        void RecvRequestHeader(vector<string> &v)
        {
            string line="X";
            while(line!="\n")
            {
                RecvOneLine(line);
                if(line!="\n")
                {//将空行读取并能够摒弃掉
                    v.push_back(line);
                }
            }
            LOG("Header Recv OK!",NORMAL);
        }
        void RecvText(string &text,int content_length)
        {
            char c;
            for(auto i=0;i<content_length;i++)
            {
                recv(sock,&c,1,0);
                text.push_back(c);
            }
        }


        //senddata
        void SendStatusLine(Http_Response *rsp)
        {
            string &sl=rsp->status_line;
            send(sock,sl.c_str(),sl.size(),0);
        }
        void SendHeader(Http_Response *rsp)
        {
            vector<string> &v=rsp->response_header;
            for(auto it=v.begin();it!=v.end();it++)
            {
                send(sock,it->c_str(),it->size(),0);
            }
        }
        void SendText(Http_Response *rsp,bool cgi_)
        {
            if(!cgi_)
            {//非Cgi方式
                string path=rsp->Path();
                int fd=open(path.c_str(),O_RDONLY);
                if(fd<0)
                {
                    LOG("open file error!",WARNING);
                    return;
                }
                sendfile(sock,fd,NULL,rsp->RecourceSize());
                close(fd);
            }
            else
            {//Cgi方式
                string &rsp_text=rsp->response_text;
                send(sock,rsp_text.c_str(),rsp_text.size(),0);
            }
        }
        ~Connect()
        {
            close(sock);
        }
};
class Entry
{
    public:
        static int ProcessCgi(Connect *conn,Http_Request *rq,Http_Response *rsp)
        {
            int input[2];    //读：0，写：1
            int output[2];
            pipe(input);    //child
            pipe(output);

            string bin=rsp->Path();   //wwwroot/a/b/binX 可执行程序的路径
            string param=rq->GetParam();    //获得参数
            string param_size="CONTENT-LENGTH=";    //知道要读多少请求正文,环境变量
            int size=param.size();
            param_size+=Util::IntToString(size);
            string &response_text=rsp->response_text;

            pid_t pid=fork();
            if(pid<0)
            {
                LOG("fork error!",ERROR);
                return 503;
            }
            else if(pid==0)
            {//child  exec*
                close(input[1]);
                close(output[0]);
                putenv((char*)param_size.c_str());
                dup2(input[0],0);
                dup2(output[1],1);

                execl(bin.c_str(),bin.c_str(),NULL);
                exit(1);    //如果替换失败
            }
            else
            {//father
                close(input[0]);
                close(output[1]);
                char c;
                for(auto i=0;i<size;++i)
                {
                    c=param[i];
                    write(input[1],&c,1);
                }
                waitpid(pid,NULL,0);


                while(read(output[0],&c,1)>0)
                {
                    response_text.push_back(c);
                }

                rsp->MakeStatusLine();
                rsp->SetRecourceSize(response_text.size());
                rsp->MakeResponseHeader();

                conn->SendStatusLine(rsp);
                conn->SendHeader(rsp);
                conn->SendText(rsp,true);
            }
            return 200;
        }
        static int  ProcessNonCgi(Connect *conn,Http_Request *rq,Http_Response *rsp)
        {
            //构建一个状态行,响应报头，相应主体
            rsp->MakeStatusLine();
            rsp->MakeResponseHeader();
            //直接用sendfile()函数将资源写入sock
            //rsp->MakeResponseText(rq);

            //发送数据
            conn->SendStatusLine(rsp);
            conn->SendHeader(rsp);   //add \n
            conn->SendText(rsp,false);
            LOG("Send Response Done!",NORMAL);
        }
        static int ProcessResponse(Connect *conn,Http_Request *rq,Http_Response *rsp)
        {
            if(rq->IsCgi())
            {
                LOG("MakeResponse Use Cgi!",NORMAL);
                ProcessCgi(conn,rq,rsp);
            }
            else
            {
                LOG("MakeResponse Use Non Cgi!",NORMAL);
                ProcessNonCgi(conn,rq,rsp);

            }
        }
        static void HandlerRequest(int sock)    //处理客户端的请求
        {
            pthread_detach(pthread_self());
            //int *sock=(int*)arg;
#ifdef _DEBUG_
            char buf[10240];
            cout<<"###############################"<<endl;
            read(sock,buf,sizeof(buf));
            cout<<buf<<endl;
            cout<<"###############################"<<endl;
#else
            //读取一行数据
            Connect *conn=new Connect(sock);
            //处理数据
            Http_Request *rq=new Http_Request;
            //构建响应
            Http_Response *rsp=new Http_Response();
            int &code=rsp->Code();
            conn->RecvOneLine(rq->request_line);
            rq->RequestLineParse();
            if(!rq->IsMethodLegal())
            {
                code=404;
                LOG("request method is not legal!",WARNING);
                goto end;
            }
            //找到了要请求的资源
            rq->UriParse();

            //判定请求的资源的路径是否合法
            if(rq->IsPathLegal(rsp)!=0)
            {
                LOG("file is not exist!",WARNING);
                code=404;
                goto end;
            }

            //读取请求的报头
            conn->RecvRequestHeader(rq->request_header);
            rq->HeaderParse();

            //是否需要继续读取，现在已经把空行读取完了
            if(rq->IsNeedRecv())
            {
                LOG("POST Method,Need Recv Begin!",NORMAL);
                //继续读取正文
                conn->RecvText(rq->request_text,rq->ContentLength());
            }

            LOG("Http Request Recv Done,OK!",NORMAL);
            //请求读完了，开始响应
            ProcessResponse(conn,rq,rsp);

end:
            delete conn;
            delete rq;
            delete rsp;

#endif
        }
};
#endif
