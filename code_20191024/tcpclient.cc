#include"tcpclient.hpp"

int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        cout<<argv[1]<<"server_ip server_port"<<endl;
        exit(1);
    }
   tcpclient *tcp = new tcpclient(argv[1],atoi(argv[2]));
   tcp->initclient();
   tcp->run();
    delete tcp;
    return 0;
}
