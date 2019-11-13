#include"selectserver.hpp"

int main(int argc,char *argv[])
{
    if(argc != 2)
    {
        cout<<"Usage:"<<argv[0]<<"port"<<endl;
        exit(0);
    }
    selectserver *sp = new selectserver(atoi(argv[1]));
    sp->Initserver();
    sp->run();
}
