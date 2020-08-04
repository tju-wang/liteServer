#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
//#include "../CGImysql/sql_connection_pool.h"
template< typename T>
class threadpool
{
public:
    threadpool(int thread_number = 8,int max_requests = 10000);
    ~threadpool();
    //向请求队列中添加任务
    bool append(T* request);
private:
    /* data */
    static void* worker(void* arg);
    void run();
private:
    int m_thread_number;    //线程池中的线程数量
    int m_max_requests;     //请求队列中允许的最大请求数
    pthread_t * m_threads;
    std::list<T*> m_workqueue;  //请求队列
    locker m_queuelocker;   //请求队列 的 锁
    sem m_queuestat;    //是否有任务需要处理 用信号量标示
    bool m_stop;    //是否结束线程
};
template< typename T>
threadpool<T>::threadpool(int thread_number = 8,int max_requests = 10000):
                    m_thread_number(thread_number),m_max_request(max_requests),mstop(false),m_threads(NULL) //默认初始化私有成员
{
    if((thread_number<=0) || (max_thread_number <= 0))  //线程池数量 或 最大线程数量<=0 break
    {
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number]; //申请线程池数组
    if(!m_threads)
    {
        throw std::exception();
    }
    for(int i=0;i<thread_number;++i)    //创建线程池中的线程
    {
        printf("create the %dth thread\n",i);
        if(pthread_create(m_threads+i,NULL,worker,this) != 0)   //pthread_create
        {
            delete [] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i]))   //改变线程为 unjoinable状态  当线程尾部遇到pthread_exit时，线程自动退出，确保资源释放
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}
template <typename T>
bool threadpool<T>::append(T* request)
{
    /*操作工作队列时，一定要加锁，因为它被所有的线程共享*/
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
void * threadpool<T>::worker(void * arg)
{
    threadpool* pool = (threadpool *)arg;
    pool->run();
    return pool;
}
template <typename T>
void threadpool<T>::run()
{
    wgile(!m_stop)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();   //request可能有问题
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request)
        {
            continue;
        }
        request->process();
    }
}



#endif

