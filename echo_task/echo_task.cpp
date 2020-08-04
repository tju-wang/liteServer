#include "echo_task.h"

locker m_lock;

//对文件描述符设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int echo_task::m_user_count = 0;
int echo_task::m_epollfd = -1;

//关闭连接，关闭一个连接，客户总量减一
void echo_task::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        printf("close %d\n", m_sockfd);
        //removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
    }
}

//初始化连接,外部调用初始化套接字地址
void echo_task::init(int sockfd, const sockaddr_in &addr)
{
    m_sockfd = sockfd;
    m_address = addr;

    addfd(m_epollfd, sockfd, true, true);
    m_user_count++;

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    m_TRIGMode = true;
    init();
}

//初始化新接受的连接
//check_state默认为分析请求行状态
void echo_task::init()
{
    // mysql = NULL;
    bytes_to_send = 0;
    // bytes_have_send = 0;
    // m_check_state = CHECK_STATE_REQUESTLINE;
    // m_linger = false;
    // m_method = GET;
    // m_url = 0;
    // m_version = 0;
    // m_content_length = 0;
    // m_host = 0;
    // m_start_line = 0;
    // m_checked_idx = 0;
    m_read_idx = 0;
    //m_write_idx = 0;
    // cgi = 0;
    // m_state = 0;
    // timer_flag = 0;
    // improv = 0;

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    //memset(m_real_file, '\0', FILENAME_LEN);
}

//循环读取客户数据，直到无数据可读或对方关闭连接
//非阻塞ET工作模式下，需要一次性将数据读完
bool echo_task::read_once()
{
    // if (m_read_idx >= READ_BUFFER_SIZE)
    // {
    //     return false;
    // }
    int bytes_read = 0;

    //LT读取数据
    if (0 == m_TRIGMode)
    {
        // bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        // m_read_idx += bytes_read;

        // if (bytes_read <= 0)
        // {
        //     return false;
        // }

        return true;
    }
    //ET读数据  将数据全部读完
    else
    {
        while (true)
        {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}

bool echo_task::write() //写到这
{
    int temp = 0;

    if (bytes_to_send == 0)
    {
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
        //init();
        return true;
    }

    while(1)
    {
        temp = writev(m_sockfd, m_iv, m_iv_count);
        if(temp<0)
        {
            printf("writev return -1\n");
            return;
        }
        else
        {
            
        }
        
    }

    // while (1)
    // {
    //     temp = writev(m_sockfd, m_iv, m_iv_count);

    //     if (temp < 0)
    //     {
    //         if (errno == EAGAIN)    //？
    //         {
    //             modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
    //             return true;
    //         }
    //         unmap();
    //         return false;
    //     }

    //     bytes_have_send += temp;
    //     bytes_to_send -= temp;
    //     if (bytes_have_send >= m_iv[0].iov_len)
    //     {
    //         m_iv[0].iov_len = 0;
    //         m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
    //         m_iv[1].iov_len = bytes_to_send;
    //     }
    //     else
    //     {
    //         m_iv[0].iov_base = m_write_buf + bytes_have_send;
    //         m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
    //     }

    //     if (bytes_to_send <= 0)
    //     {
    //         unmap();
    //         modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);

    //         if (m_linger)
    //         {
    //             init();
    //             return true;
    //         }
    //         else
    //         {
    //             return false;
    //         }
    //     }
    // }
}
