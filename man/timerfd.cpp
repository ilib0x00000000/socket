#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

/**
 * 创建一个定时器对象，返回一个文件描述符
 * int timerfd_create(int clockid, int flags);
 * clockid: CLOCK_REALTIME/CLOCK_MONOTONIC
 *      CLOCK_REALTIME: 系统实时时间，计时器随系统时间改变而改变
 *      CLOCK_MONOTONIC: 从计时器计时开始，不再随系统时间变化而改变
 * flags: TFD_NONBLOCK/TFD_CLOEXEC
 *      TFD_NONBLOCK: 非阻塞模式
 *      TFD_CLOEXEC: 在exec时关闭文件描述符
 *
 *
 * 设置新的超时时间，并开始计时，能够启动和停止定时器
 * int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
 * struct timespec{
 *      time_t  tv_sec;               // 秒
 *      long    tv_nsec;              // 纳秒
 * };
 * struct itimerspec{
 *      struct timespec it_interval;  // 定时间隔周期     不为0表示周期性定时器
 *      struct timespec it_value;     // 第一次超时时间
 * };
 * it_interval和it_value都为0表示停止计时器
 *
 * 获取距离下次超时剩余的时间
 * int timerfd_gettime(int fd, struct itimerspec *curr_value);
 */

#define EPOLL_LISTEN_CNT     256
#define EPOLL_LISTEN_TIMEOUT 500
static int g_epollfd = -1;
static int g_timerfd = -1;
uint64_t   tot_exp = 0;

int epoll_init() 
{
    int epfd;

    epfd = epoll_create(EPOLL_LISTEN_CNT);
    if(epfd < 0)
    {
        return -1;
    }
    g_epollfd = epfd;

    return epfd;
}

int epoll_add_fd(int fd)
{
    int ret;
    struct epoll_event event;

    memset(&event, 0, sizeof(event));
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;

    ret = epoll_ctl(g_epollfd, EPOLL_CTL_ADD, fd, &event);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

int timerfd_init()
{
    int timerfd;
    int ret;
    struct itimerspec new_value;

    // it_interval不为0表示周期性定时器
    new_value.it_interval.tv_sec = 2;
    new_value.it_interval.tv_nsec = 0; 

    // 每隔3s 触发一次定时器
    new_value.it_value.tv_sec = 3;
    new_value.it_value.tv_nsec = 0;

    timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd < 0)
    {
        return -1;
    }

    // 设置定时器
    ret = timerfd_settime(timerfd, 0, &new_value, NULL);
    if(ret < 0)
    {
        return -1;
    }

    epoll_add_fd(timerfd);
    g_timerfd = timerfd;

    return timerfd;
}

void timerfd_handle(int fd)
{
    uint64_t x;
    tot_exp += 1;
    printf("定时器第 %02d 次触发-------", tot_exp);
    read(fd, &x, sizeof(x));
    printf("x的值是： %d\n", x);
}

void epoll_event_handle()
{
    int i = 0;
    int sfd;
    int fd_cnt = 0;
    struct epoll_event events[EPOLL_LISTEN_CNT];

    memset(events, 0, sizeof(events));
    while(1)
    {
        fd_cnt = epoll_wait(g_epollfd, events, EPOLL_LISTEN_CNT, 0);
        for(i=0; i<fd_cnt; i++)
        {
            sfd = events[i].data.fd;
            if(sfd == g_timerfd)
            {
                timerfd_handle(sfd);
            }
        }
    }
}

int main()
{
    if(epoll_init() < 0) {
        return -1;
    }

    if(timerfd_init() < 0) {
        return -1;
    }

    epoll_event_handle();
    
    return 0;
}
