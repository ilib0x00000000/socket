//
// Created by ilib0x00000000 on 17-9-12.
//

#ifndef PROCESSPOOL_PCSPOOL_H
#define PROCESSPOOL_PCSPOOL_H

#include <iostream>
#include <sched.h>

/**
 * 线程相关信息
 */
class Process
{
public:
    Process(): m_pid(-1)
    {}
public:
    int   m_pipefd[2];
    pid_t m_pid;
};


// 模板类
template <typename T>
class ProcessPool
{
private:
    // 将构造函数私有化，只能使用后面的静态函数才能创建ProcessPool实例
    ProcessPool(int listenfd, int process_number=8);

public:
    // 保证程序只有一个进程池实例
    static ProcessPool<T>* create(int listenfd, int process_number=8);
    ~ProcessPool();
    void run();

private:
    void setup_sig_pipe();
    void run_parent();
    void run_child();

private:
    static const int MAX_PROCESS_NUMBER = 16;   // 进程池允许的最大的子进程数量
    static const int USER_PER_PROCESS = 65536;  // 每个子进程最多可处理的客户连接数
    static const int MAX_EVENT_NUMBER = 10000;  // epoll最多能处理的事件数
    int m_process_number;     // 进程池中的进程总数
    int m_index;              // 子进程在池中的序号
    int m_epolled;            // 每个子进程都有一个epoll内核事件表 用m_epollfd标识
    int m_listenfd;           // 监听socket
    int m_stop;               // 子进程通过m_stop来决定是否终止运行
    Process* m_sub_process;   // 保存所有子进程的描述信息
    static ProcessPool<T>* m_instance;  // 进程池静态实例
};

#endif //PROCESSPOOL_PCSPOOL_H
