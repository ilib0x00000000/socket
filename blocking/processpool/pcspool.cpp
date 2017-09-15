//
// Created by ilib0x00000000 on 17-9-12.
//

#include <assert.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include "pcspool.h"

static int sig_pipefd[2];    // 管道

// 信号处理方式
static void sig_handle(int signo)
{
    int save_errno;

    save_errno = errno;

    send(sig_pipefd[1], "1", 1, 0);   // 向管道写入一个字节信息

    errno = save_errno;
}


// 设置非阻塞
static int setnonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags|O_NONBLOCK);
}

// 将文件描述符添加到epoll句柄所指向的红黑树
static void add_fd(int epfd, int fd)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
}


static void add_sig(int signo, void (*handler)(int), bool restart= true)
{
    int ret;
    struct sigaction sa;     // 信号处理器

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handler;
    if(restart)
    {
        sa.sa_flags |= SA_RESTART;
    }

    sigfillset(&sa.sa_mask);    // 信号掩码
    ret = sigaction(signo, &sa, NULL);   // 设置信号处理方式
    assert(ret != -1);

    return;
}


template <typename T>
ProcessPool<T>* ProcessPool<T>::m_instance = NULL;


// 进程池构造函数
// process_number指定了进程池中的进程数量
template <typename T>
ProcessPool<T>::ProcessPool(int listenfd, int process_number) :m_listenfd(listenfd), m_process_number(process_number),
    m_index(-1), m_stop(false)
{
    assert((process_number>0) && (process_number<=MAX_PROCESS_NUMBER));
    m_sub_process = new Process[process_number];   // 子进程数组信息
    assert(m_sub_process);

    // 创建process_num个子进程，并建立它们和父进程之间的管道
    for(int i=0; i<process_number; i++)
    {
        int ret = socketpair(PF_UNIX, SOCK_STREAM, 0,m_sub_process[i].m_pipefd);
        assert(ret == 0);

        m_sub_process[i].m_pid = fork();
        assert(m_sub_process[i].m_pid >= 0);

        if(m_sub_process[i].m_pid > 0)
        {
            // 父进程
            // 关闭父进程中的写端
            close(m_sub_process[i].m_pipefd[1]);
            continue;
        }else
        {
            // 子进程    注意这里一定要有break;   不然子进程中也会继续fork() 创建新的子进程
            // 关闭子进程中的读端
            close(m_sub_process[i].m_pipefd[0]);
            m_index = i;
            break;
        }
    }
}


// 统一事件源
template <typename T>
void ProcessPool<T>::setup_sig_pipe()
{
    m_epolled = epoll_create(5);   // 创建一个epoll句柄
    assert(m_epolled != -1);

    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);

    setnonblocking(sig_pipefd[1]);      // 设置写非阻塞

    add_fd(m_epolled, sig_pipefd[0]);   // 将管道的读端添加到epoll句柄指向的红黑树中

    add_sig(SIGCHLD, sig_handle);     // 设置信号的处理函数， sig_handle收到对应的信号之后，向管道中写入一个字符
    add_sig(SIGTERM, sig_handle);
    add_sig(SIGINT,  sig_handle);
    add_sig(SIGPIPE, SIG_IGN);     // 忽略这个新号

}

// 父进程中的m_index值为-1，子进程中的m_index值>=0
template <typename T>
void ProcessPool<T>::run()
{
    if(m_index != -1)
    {
        run_child();  // 运行子进程
        return ;
    }

    run_parent();    // 运行父进程
    return;
}

// 子进程执行的任务
template <typename T>
void ProcessPool<T>::run_child()
{
    setup_sig_pipe();


}


// 父进程执行的任务
template <typename T>
void ProcessPool<T>::run_parent() {}
