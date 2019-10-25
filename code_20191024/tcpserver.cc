#include"tcpserver.hpp"

int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        cout<<argv[0]<<"IP PORT"<<endl;
        exit(1);
    }
    tcpserver *tsp = new tcpserver(argv[1],atoi(argv[2]));
    tsp->initserver();

    tsp->start();
    delete tsp;
}
