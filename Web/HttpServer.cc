#include"HttpServer.hpp"
#include<iostream>

using namespace std;

static void Usage(std::string proc)
{
    cout<<"Usage:"<<proc<<" port"<<endl;
}
int main(int argc,char* argv[])
{
    if(argc!=2)    //证明参数传递有误，输出使用手册
    {
        Usage(argv[0]);    //argv[0],程序的名称
        exit(1);
    }
    HttpServer *ser=new HttpServer(atoi(argv[1]));
    ser->InitServer();
    ser->Start();
    delete ser;
    return 0;
}
