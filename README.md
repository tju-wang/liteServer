# liteServer
A project for learning linux program

1. 使用epoll IO复用，实现echo服务器的框架
2. 使用线程池，readtor模式，将主线程于与工作线程解耦，实现任务处理
 * 要先实现对信号处理的封装
 * 使用Linux高性能服务器编程书上的代码
 * 当前存在的问题是，启动线程池后的数据读取、写入在哪里  怎么写  捋一捋思路
3. 

