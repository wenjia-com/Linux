#ifndef __HTTPSERVER_HPP__
#define __HTTPSERVER_HPP__

#include<iostream>
#include"ProtocolUtil.hpp"
#include"ThreadPool.hpp"

class HttpServer
{
    public:
        HttpServer(int port_):port(port_),listen_sock(-1),pool(5)
        {}
        void InitServer()
        {
            listen_sock=SocketApi::Socket();
            SocketApi::Bind(listen_sock,port);
            SocketApi::Listen(listen_sock);
            pool.InitThreadPool();
        }
        /*
        void HandlerRequest(void *arg)
        {
            pthread_detach(pthread_self());    //将自身线程分离
            int sock=(*int)arg;
            delete arg;
        }*/
        void Start()
        {
            for(;;)
            {
                string peer_ip;
                int peer_port;
                int sock=SocketApi::Accept(listen_sock,peer_ip,peer_port);
                if(sock>=0)
                {
                    cout<<"get a new link......"<<endl;
                    cout<<peer_ip<<" : "<<peer_port<<endl;
                    //创建一个线程，用以等待客户端请求
                    //pthread_t tid;
                    //pthread_create(&tid,NULL,Entry::HandlerRequest,(void*)sockp);
                    Task t(sock,Entry::HandlerRequest);
                    pool.PushTask(t);
                }
            }
        }
        ~HttpServer()
        {
            if(listen_sock>=0)
            {
                close(listen_sock);
            }
        }
    private:
        int listen_sock;    //监听
        int port;    //端口号
        ThreadPool pool;    //线程池
};

#endif
