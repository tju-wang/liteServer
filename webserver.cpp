#include "webserver.h"


void WebServer::thread_pool()
{
    //线程池
    m_pool = new threadpool<http_conn>(m_actormodel, m_thread_num);
}

WebServer::WebServer()  //类的构造函数
{
    
    //http_conn类对象
    //users = new http_conn[MAX_FD];
    
    //root文件夹路径
    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);
    printf("cwd %s",m_root);
    //定时器
    //users_timer = new client_data[MAX_FD];
}

WebServer::~WebServer() //析构函数
{
    close(m_epollfd);
    //close(m_listenfd);
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    // delete[] users;
    // delete[] users_timer;
    // delete m_pool;
}

void WebServer::init(int port, string user, string passWord, string databaseName, int log_write, 
                     int opt_linger, int trigmode, int sql_num, int thread_num, int close_log, int actor_model)
{
    m_port = port;
    m_user = user;    //用于http连接的
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_log_write = log_write;
     m_OPT_LINGER = opt_linger;
    m_TRIGMode = trigmode;
    m_close_log = close_log;
    m_actormodel = actor_model;
}

void WebServer::trig_mode() //设置触发模式
{
    //LT + LT
    if (0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
    //LT + ET
    else if (1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    //ET + LT
    else if (2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    //ET + ET
    else if (3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}



void WebServer::eventListen()
{
    //网络编程基础步骤
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    //优雅关闭连接
    // if (0 == m_OPT_LINGER)  //LINGER设置
    // {
    //     struct linger tmp = {0, 1};
    //     setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    // }
    // else if (1 == m_OPT_LINGER)
    // {
    //     struct linger tmp = {1, 1};
    //     setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    // }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;   //IPv4
    address.sin_addr.s_addr = htonl(INADDR_ANY);    //host to network long  主机字节序转换为网络字节序  INADDR_ANY是本机IP 0.0.0.0
    address.sin_port = htons(m_port);   //服务器要使用的port

    int flag = 1;
    setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));  //允许port完成重复捆绑
    ret = bind(m_listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret>=0);
    ret = listen(m_listenfd,5);
    assert(ret>=0);

    // utils.init(TIMESLOT);    //定时器相关
    //epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);    //建议值
    assert(m_epollfd != -1);
    
    utils.addfd(m_epollfd,m_listenfd,false,m_LISTENTrigmode);
    // http_conn::m_epollfd = m_epollfd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);    //创建双向通讯的一对管道
    assert(ret != -1);
    utils.setnonblocking(m_pipefd[1]);
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);  //epoll监视管道

//!信号处理  要加的
    //utils.addsig(SIGPIPE, SIG_IGN);
    // utils.addsig(SIGALRM, utils.sig_handler, false);
    //utils.addsig(SIGTERM, utils.sig_handler, false);

    //alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;

}   //之后的事情  监听连接放到epoll events里边


void WebServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;
    while(!stop_server)
    {
        int number = epoll_wait(m_epollfd,events,MAX_EVENT_NUMBER,-1);
        if(number<0 && errno!=EINTR)
        {
            cout << "epoll failure";
            break;
        }

        for(int i=0;i<number;++i)
        {
            int sockfd = events[i].data.fd;

            if(sockfd == m_listenfd)
            {
                //bool flag = dealclientdata();
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);    //LT模式，只处理了一个连接
                utils.addfd(m_epollfd,connfd,false,m_LISTENTrigmode);    //加入epoll监控的内核链表

                //users[connfd].init(connfd, client_address, m_root, m_CONNTrigmode, m_close_log, m_user, m_passWord, m_databaseName);
            }
            // else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            // {
            //     //服务器端关闭连接，移除对应的定时器
            //     util_timer *timer = users_timer[sockfd].timer;
            //     deal_timer(timer, sockfd);
            // }
            //处理信号
            // else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN))
            // {
            //     bool flag = dealwithsignal(timeout, stop_server);
            //     if (false == flag)
            //         //LOG_ERROR("%s", "dealclientdata failure");
            // }
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                //dealwithread(sockfd);
                char *buff = (char *)malloc(sizeof(char)*100);
                bzero(buff,100);
                int len = read(sockfd,buff,100);
                buff[len] = '\0';
                printf("read len = %d str =  %s\n",len,buff);
                write(sockfd,buff,len);

            }
            else if (events[i].events & EPOLLOUT)
            {
                //dealwithwrite(sockfd);
                printf("write: \n");
            }
        }
        // if (timeout)
        // {
        //     utils.timer_handler();

        //     LOG_INFO("%s", "timer tick");

        //     timeout = false;
        // }
        }
}


/*
void WebServer::dealwithread(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;

    //reactor
    if (1 == m_actormodel)
    {
        // if (timer)
        // {
        //     adjust_timer(timer);
        // }

        //若监测到读事件，将该事件放入请求队列
        m_pool->append(users + sockfd, 0);

        while (true)
        {
            if (1 == users[sockfd].improv)
            {
                if (1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (users[sockfd].read_once())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            m_pool->append_p(users + sockfd);

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
}
*/
