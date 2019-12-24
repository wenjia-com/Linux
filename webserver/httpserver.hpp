#pragma once

#include<iostream>
#include<pthread.h>
#include"util.hpp"
#include"protocol.hpp"



using namespace std;


class Sock
{
    private:
        int port;
        int sock;
    public:
        Sock(const int &port_):port(port_),sock(-1)
        {

        }
        void Socket()
        {
            sock = socket(AF_INET,SOCK_STREAM,0);
            if(sock<0)
            {
                cerr<<"socket error"<<endl;
                exit(2);
            }
            int opt = 1;
            setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        }
        void Bind()
        {
            struct sockaddr_in local;
            local.sin_family = AF_INET;
            local.sin_addr.s_addr = htonl(INADDR_ANY);
            local.sin_port = htons(port);
            if(bind(sock,(struct sockaddr *)&local,sizeof(local))<0)
            {
                cerr<<"bind error"<<endl;
                exit(3);
            }
        }
        void Listen()
        {
            if(listen(sock,10)<0)
            {
                cerr<<"listen error "<<endl;
                exit(4);
            }
        }
        int Accept()
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int fd = accept(sock,(struct sockaddr*)&peer,&len);
            if(fd<0)
            {
                cerr<<"accpet error "<<endl;
                return -1;
            }
            cout<<"get a new link..."<<endl;
            return fd;
        }
            
        ~Sock()
        {
            if(sock>=0)
            {
                close(sock);
            }
        }
        
};

#define DEFAULT_PORT 8080
class httpserver
{
    private:
        Sock sock;
    public:
        httpserver(int port = DEFAULT_PORT):sock(port)
        {
        }
        void inithttpserver()
        {
            sock.Socket();
            sock.Bind();
            sock.Listen();
        }
        void start()
        {
            for(;;)
            {
                int sock_=sock.Accept();
                if(sock_>=0)
                {
                    pthread_t tid;
                    int *p = new int(sock_);
                    pthread_create(&tid,nullptr,Entry::HandlerRequest,p);
                }
            }
        }
};
