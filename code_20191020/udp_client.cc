#include"udp_client.hpp"
using namespace std;
void Usage(string proc)
{
    cout<<proc<<"server_ip server_port"<<endl;
}
int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }
    Client *ct = new Client(argv[1],atoi(argv[2]));
    ct->Initclient();
    ct->Run();
    delete ct;
}
