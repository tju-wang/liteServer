#ifndef ECHO_TASK_H
#define ECHO_TASK_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include "../lock/locker.h"

class echo_task
{
public:
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    static int m_epollfd;
    static int m_user_count;

private:
    /* data */
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_TRIGMode;
    int m_read_idx;
    int bytes_to_send;
    struct stat m_file_stat;    //？
    struct iovec m_iv[2];
    int m_iv_count;

    void init();

public:
    echo_task();
    ~echo_task();

    void init(int sockfd, const sockaddr_in &addr);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();   //循环读取客户数据，直到无数据可读或对方关闭连接
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }
};

echo_task::echo_task(/* args */)
{
}

echo_task::~echo_task()
{
}




#endif
