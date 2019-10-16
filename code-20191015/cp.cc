#include<iostream>
#include<pthread.h>
#include<queue>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>


using namespace std;


class BlockQueue{
    private:
        queue<int>q;
        int cap;
        pthread_mutex_t lock;
        
        pthread_cond_t c_cond;
        pthread_cond_t p_cond;


    private:

            void  LockQueue()
            {
                pthread_mutex_lock(&lock);


            }
            void UnlockQueue()
            {
                pthread_mutex_unlock(&lock);
            }
            bool QueueIsFull()
            {
                return q.size() == cap;
            }

            bool QueueIsEmpty()
            {
                return q.size() == 0;
            }
            void ProductWait()
            {
                cout<<"product wait..."<<endl;
                pthread_cond_wait(&p_cond,&lock);
            }
            void ConsumerWait()
            
            {
                cout<<"consuner wait..."<<endl;
                pthread_cond_wait(&c_cond,&lock);
            }
            void SignalConsumer()
            {
                cout<<"signal consumer C..."<<endl;
                pthread_cond_signal((&c_cond));
            }
            void SignalProduter()
            {
                cout<<"signal producter p ..."<<endl;
                pthread_cond_signal(&p_cond);
            }
    public:
        BlockQueue(int cap_=32):cap(cap_)
        {
            pthread_mutex_init(&lock,NULL);
            pthread_cond_init(&c_cond,NULL);
            pthread_cond_init(&p_cond,NULL);
        }
        //product
        void PushDate(int date)
        {
            LockQueue();
            if(QueueIsFull())
            {
                cout<<"queue is fill"<<endl;
                SignalConsumer();
                ProductWait();

            }
            q.push(date);
            UnlockQueue();

        }
        void PopDate(int &out)
        {
            LockQueue();
            if(QueueIsEmpty())
            {
                cout<<"queue is empty"<<endl;
                SignalProduter();
                ConsumerWait();
            }
            out=q.front();
            q.pop();
            UnlockQueue();

        }


        ~BlockQueue()
        {
            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&c_cond);
            pthread_cond_destroy(&p_cond);
        }

};

void *product(void *arg)
{
    BlockQueue *bq = (BlockQueue*)arg;
    while(1)
    {
        int date = rand()%100+1;
        bq->PushDate(date);
        cout<<"product date is:"<<date<<endl;
        sleep(1);
    }
}
void *consumer(void *arg)
{
    int date;
    BlockQueue *bq = (BlockQueue*)arg;
    while(1)
    {
        bq->PopDate(date);
        cout<<"consumer date is:"<<date<<endl;
    }
}

int main()
{
        srand((unsigned long)time(NULL));

        BlockQueue *bq = new BlockQueue(4);
        pthread_t c,p;
        pthread_create(&c,NULL,consumer,(void*)bq);
        pthread_create(&p,NULL,product,(void*)bq);


        pthread_join(p,NULL);
        pthread_join(c,NULL);


}
