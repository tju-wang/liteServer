
#include <iostream>
using namespace std;
#include "webserver.h"
#include "config.h"


int main(int argv,char* argc[])
{
    //需要修改的数据库信息,登录名,密码,库名
    string user = "root";
    string passwd = "root";
    string databasename = "qgydb";

    cout << "Hi liteWebserver start!\n";

    Config config;
    config.parse_arg(argv,argc);

    WebServer server;
    server.init(config.PORT, user, passwd, databasename, config.LOGWrite, 
                config.OPT_LINGER, config.TRIGMode,  config.sql_num,  config.thread_num, 
                config.close_log, config.actor_model);

    //触发模式
    server.trig_mode();

    //监听  实现echo
    server.eventListen();

    //epoll事件统一处理 监听、连接、信号处理、read write等
    server.eventLoop();

    return 0;
}

