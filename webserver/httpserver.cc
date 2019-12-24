#include"httpserver.hpp"

static void Usage(string proc)
{
    cout<<"Usage:"<<proc<<"port"<<endl;
}
int main(int argc ,char *argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    httpserver *htp = new httpserver(atoi(argv[1]));

    htp->inithttpserver();
    htp->start();
    delete htp;
    return 0;
}
