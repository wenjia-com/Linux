#include<stdio.h>
#include<unistd.h>
int main()
{
    pid_t id = fork();
    if(id<0)
    {
        perror("fork");
        return 1;
    }
    else if(id == 0)
    {
        while(1)
        {
            printf("I am child : pid : %d,ppid :%d\n",\
                    getpid(),getppid());
            sleep(1);
        }
    }
    else
    {
        while(1)
        {
            printf("I am father : pid : %d,ppid : %d\n",\
                    getpid(),getppid());
            sleep(2);
        }
    }
    return 0;
}
