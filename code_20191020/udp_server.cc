#include"udp_server.hpp"

int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        cout<<argv[0]<<"IP"<<"PORT "<<endl;
        exit(1);
    }
    Server *sp = new Server(argv[1],atoi(argv[2]));
    sp->InitServer();
    sp->Start();
    delete sp;
}
