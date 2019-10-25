#pragma once
#include<iostream>
#include<string>
#include<cstring>
#include<unistd.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>





using namespace std;

class tcpserver
{
    private:
        int listen_sock;
        string ip;
        int port;
    public:

        tcpserver(string ip_ = "127.0.0.1",int port_ = 8888):ip(ip_),port(port_)
        {

        }
        void initserver()
        {
            listen_sock = socket(AF_INET,SOCK_STREAM,0);
            if(listen_sock <0)
            {
                cerr<<"listen_socket error"<<endl;
                exit(2);
            }
            struct sockaddr_in local;
            local.sin_family = AF_INET;
            local.sin_addr.s_addr = inet_addr(ip.c_str());
            local.sin_port = htons(port);
            if(bind(listen_sock,(struct sockaddr *)&local,sizeof(local))<0)
            {
                cerr<<"bind error"<<endl;
                exit(3);
            }
            if(listen(listen_sock,5)<0)
            {
                cerr<<"listen error"<<endl;
                exit(4);
            }

        }
        void serverviceio(int sock)
        {
            char buf[1024];
            while(1)
            {
                ssize_t s = read(sock,buf,sizeof(buf)-1);
                if(s >0)
                {
                    buf[1024] = 0;
                    cout<<"client# "<<buf<<endl;
                    write(sock,buf,strlen(buf));

                }
                else if(s == 0)
                {
                    cout<<"client quit..."<<endl;
                    break;
                }
                else
                {
                    cerr<<"read error!"<<endl;
                }

            }
        }
        void start()
        {
            struct sockaddr_in peer;
            while(1)
            {
                socklen_t len = sizeof(peer);
                int sock = accept(listen_sock,(struct sockaddr *)&peer,&len);
                if(sock < 0)
                {
                    cerr<<"accept error"<<endl;
                    continue;
                
                }
                serverviceio(sock);
                close(sock);
            }
        }
        ~tcpserver()
        {

        }
};
