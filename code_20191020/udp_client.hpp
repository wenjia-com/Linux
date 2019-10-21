#pragma once
#include<iostream>
#include<string>
#include<cstring>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>

using namespace std;
class Client{
    private:
        int sock;
        string peer_ip;
        int peer_port;
    public:
        Client(string ip,int port):peer_ip(ip),peer_port(port)
        {
            sock = -1;
        }
        void Initclient()
        {
            sock = socket(AF_INET,SOCK_DGRAM,0);
            if(sock < 0)
            {
                cerr<<"socket error..."<<endl;
                exit(2);
            }
        }
        void Run()
        {
            bool quit = false;
            string message;
            char buf[1024];
            while(!quit)
            {
                struct sockaddr_in peer;
                peer.sin_family = AF_INET;
                peer.sin_port =htons(peer_port);
                peer.sin_addr.s_addr = inet_addr(peer_ip.c_str());
                socklen_t len = sizeof(peer);
                cout<<"Please Enter#";
                cin>>message;
                sendto(sock,message.c_str(),message.size(),0,(struct sockaddr*)&peer,sizeof(peer));
                ssize_t s = recvfrom(sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&peer,&len);
                    if(s>0)
                    {
                        buf[s] = 0;
                        cout<<"Server echo# "<<buf<<endl;
                    }
            }
        }
        ~Client()
        {

        }
 };
