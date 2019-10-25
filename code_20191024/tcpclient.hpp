#pragma once

#include<iostream>
#include<string>
#include<cstring>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include<unistd.h>



using namespace std;


class tcpclient
{
    private:
        int sock;
        string ip;
        int port;
    public:
    tcpclient(string ip_ = "127.0.0.1",int port_ = 8888):ip(ip_),port(port_)
    {
        sock = -1;
    }
    void initclient()
    {
        sock = socket(AF_INET,SOCK_STREAM,0);
        if(sock<0)
        {
            cerr<<"socket error"<<endl;
            exit(2);
        }

    }
    void run()
    {
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(ip.c_str());
        server.sin_port = htons(port);
        if(connect(sock,(struct sockaddr *)&server,sizeof(server))<0)
        {
            cerr<<"connest error"<<endl;
            exit(3);

        }
        char buf[1024];
        string massage;
        while(1)
        {
            cout<<"please enter# ";
            cin>>massage;
            write(sock,massage.c_str(),massage.size());
            ssize_t s = read(sock,buf,sizeof(buf)-1);
            if(s > 0)
            {
                buf[s] = 0;
                cout<<"server echo#"<<buf<<endl;
            }
        }
    }
    ~tcpclient()
    {
        if(sock > 0)
        
        {
            close(sock);
        }

    }

};
