#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__
#include<iostream>
#include<queue>
#include<pthread.h>

typedef void (*handler_t)(int);    //函数指针类型

class Task
{
    private:
        int sock;
        handler_t handler;
    public:
        Task(int sock_,handler_t handler_):sock(sock_),handler(handler_)
        {}
        void Run()
        {
            handler(sock);
        }
        ~Task()
        {}
};

class ThreadPool
{
    private:
        int num;   //线程数量
        int idle_num;
        queue<Task> task_queue;   //任务队列
        pthread_mutex_t lock;
        pthread_cond_t cond;    //唤醒线程
    public:
        ThreadPool(int num_):num(num_),idle_num(0)
        {
            pthread_mutex_init(&lock,NULL);
            pthread_cond_init(&cond,NULL);
        }
        void LockQueue()
        {
            pthread_mutex_lock(&lock);
        }
        void UnLockQueue()
        {
            pthread_mutex_unlock(&lock);
        }
        void Idle()    //线程等待
        {
            idle_num++;
            pthread_cond_wait(&cond,&lock);
            idle_num--;
        }
        void Wakeup()
        {
            pthread_cond_signal(&cond);
        }
        Task PopTask()
        {
            Task t=task_queue.front();
            task_queue.pop();
            return t;
        }
        void PushTask(Task t)
        {
            LockQueue();
            task_queue.push(t);
            UnLockQueue();
            Wakeup();
        }
        static void *ThreadRoutine(void *arg)   //处理常规事务
        {
            pthread_detach(pthread_self());
            ThreadPool *tp=(ThreadPool*)arg;
            while(true)
            {
                tp->LockQueue();
                while(tp->task_queue.empty())
                {
                    tp->Idle();
                }
                Task t=tp->PopTask();
                tp->UnLockQueue();
                cout<<"task is handler by: "<<pthread_self()<<endl;
                t.Run();
            }
        }
        void InitThreadPool()
        {
            pthread_t tid;
            for(int i=0;i<num;++i)
            {
                pthread_create(&tid,NULL,ThreadRoutine,this);
            }
        }
        ~ThreadPool()
        {
            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&cond);
        }

};

class signleton
{
    private:
        static ThreadPool *p;
    public:
        static ThreadPool *GetInstance()
        {
            if(p==NULL)
            {
                p=new ThreadPool(5);
                p->InitThreadPool();
            }
            return p;
        }
};
#endif
