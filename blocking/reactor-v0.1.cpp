#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cassert>
#include <iostream>


/**
 * @author:       ilib0x00000000
 * @email:        ilib0x00000000@aliyun.com
 * @date:         17-9-4
 * @os:           ubuntu 16.04.4
 * @c compiler:   gcc -v 5.4.0
 * @c++ compiler: g++ -v 5.4.0
 * @github:       https://github.com/ilib0x00000000
 */

#define BUFF_SIZE   4096
#define MAX_EVENTS  1024
#define SERVER_PORT 6666

/**
 * 描述符相关信息
 */
struct event_s
{
    int fd;
    int events;
    int status;  // 是否在epfd句柄指向的数上
    void *arg;
    void (*call_back)(int fd, int events, void *arg);    // 回调函数指针
    char buff[BUFF_SIZE];
    int len;
    long last_active;
};


// 回调函数
void cb_accept(int fd, int events, void *arg);
void cb_read(int fd, int events, void *arg);
void cb_write(int fd, int events, void *arg);


// 事件操作
void event_set(struct event_s *ev, int fd, void (*call_back)(int, int, void *), void *arg);
void event_add(int epfd, int events, struct event_s *ev);
void event_del(int epfd, struct event_s *ev);

int make_socket_nonblocking(int sockfd);
int init_listen_socket(int epfd, int port);

// 主事件循环
void loop();


int epfd;                               // 全局变量 epoll指向的文件句柄
struct event_s gevents[MAX_EVENTS+1];   // 监听的文件描述符数组


void main()
{
    loop();
}

void loop()
{
    int i;
    int nready;
    int result;
    long now;
    struct epoll_event events[MAX_EVENTS+1];  // epoll_wait()返回时使用

    epfd = epoll_create(MAX_EVENTS+1);        // 创建一个epfd
    assert(epfd >= 0);

    // 初始化监听套接字
    result = init_listen_socket(epfd, SERVER_PORT);
    assert(result >= 0);

    int checkpos = 0;

    while(1)
    {
        now = time(NULL);

        // 每次检测100个对象是否超时
       for(i=0; i<100; i++)
       {
           if(checkpos == MAX_EVENTS)
               checkpos = 0;

           if(gevents[checkpos].status != 1)
               continue;

           long duration = now-gevents[checkpos].last_active; // 客户端不活跃的时间
           if(duration >= 60)
           {
               /**
                * 断开不活跃的连接
                */
               close(gevents[checkpos].fd);   // 关闭套接字
               event_del(epfd, &gevents[checkpos]);
           }
           checkpos ++;
       }

        // 1s轮询一次
        nready = epoll_wait(epfd, events, MAX_EVENTS+1, 1000);
        if(nready < 0)
        {
            std::cout << "epoll_wait调用出错" << std::endl;
            break;
        }

        for(i=0; i<nready; i++)
        {
            int fdts = events[i].data.fd;
            char *pc = (char *)events[i].data.ptr;
            struct event_s *es = (struct event_s *)events[i].data.ptr;


            if((events[i].events & EPOLLIN) && (es->events & EPOLLIN))
            {
                es->call_back(es->fd, events[i].events, es->arg);
            }

            if((events[i].events & EPOLLOUT) && (es->events & EPOLLOUT))
            {
                es->call_back(es->fd, events[i].events, es->arg);
            }
        }
    }
}


void cb_accept(int fd, int events, void *arg)
{
    int i;
    int sockfd;
    int result;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    sockfd = accept(fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);
    if(sockfd < 0)
    {
        // accept是慢系统调用，需要对errno做检查
        if(errno == EAGAIN || errno == EINTR)
        {
            ; // goto accept
        } else
        {
            ; // 暂时不做任何处理
        }
    }

    // 设置新连接套接字为非阻塞
    result = make_socket_nonblocking(sockfd);
    if(result < 0)
    {
        close(sockfd);
        return ;
    }


    for(i=0; i<MAX_EVENTS; i++)
    {
        if(gevents[i].status == 0)
        {
            event_set(&gevents[i], sockfd, cb_read, &gevents[i]);
            event_add(epfd, EPOLLIN, &gevents[i]);
        }
    }

    if(i==MAX_EVENTS)
    {
        close(sockfd);
        // log
        return ;
    }
}

void cb_read(int fd, int events, void *arg)
{
    int n;

    struct event_s *dtsrg = reinterpret_cast<struct event_s *>(arg);
    char *buff = dtsrg->buff;

READ_AGAIN:
    // 一次非阻塞读取4KB
    n = read(fd, buff, BUFF_SIZE-1);

    if(n < 0)
    {
        if(errno == EINTR || errno==EAGAIN || errno==EWOULDBLOCK)
        {
            goto READ_AGAIN;
        } else
        {
            // 读出错
            close(fd);
            event_del(epfd, dtsrg);
        }
    }else
    {
        dtsrg->len = n;
        dtsrg->buff[n] = '\0';

        event_set(dtsrg,fd, cb_write, dtsrg);
        event_add(epfd, EPOLLOUT, dtsrg);
    }
    return ;
}

void cb_write(int fd, int events, void *arg)
{
    int i;
    int nw;
    struct event_s *ev;

    ev = reinterpret_cast<struct event_s *>(arg);

    // 小写转大写
    for(i=0; i<ev->len; i++)
    {
        ev->buff[i] = toupper(ev->buff[i]);
    }

WRITE_AGAIN:
    nw = write(fd, ev->buff, ev->len);
    if(nw < 0)
    {
        if(errno==EINTR || errno==EAGAIN || errno==EWOULDBLOCK)
        {
            goto WRITE_AGAIN;
        }
    }

    close(fd);
    event_del(epfd, ev);
}


void event_del(int epfd, struct event_s *ev)
{
    struct epoll_event evt;
    evt.events = ev->events;
    evt.data.ptr = ev->arg;

    if(ev->status != 1)
        return ;

    epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &evt);

    close(ev->fd);
    memset(ev, 0, sizeof(struct event_s));
}

void event_add(int epfd, int events, struct event_s *sev)
{
    int op;
    int result;
    struct epoll_event ev;

    ev.events = sev->events = events;
    ev.data.ptr = sev;


    if(sev->status == 1)
    {
        op = EPOLL_CTL_MOD;      // 已经在树上
    }else
    {
        op = EPOLL_CTL_ADD;      // 将其加到红黑树上
        sev->status = 1;
    }

    result = epoll_ctl(epfd, op, sev->fd, &ev);
    if(result < 0)
    {
        perror("event_add::epoll_ctl");
        close(sev->fd);
        // event_del();
    }
}

void event_set(struct event_s *sev, int fd, void (*call_back)(int, int, void *), void *arg)
{
    sev->fd = fd;
    sev->call_back = call_back;
    sev->events = 0;
    sev->status = 0;
    sev->arg = arg;
    sev->last_active = time(NULL);
}

int init_listen_socket(int epfd, int port)
{
    int result;
    int listen_fd;
    struct sockaddr_in server_addr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定主机地址
    result = bind(listen_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr));
    assert(result >= 0);

    // 设置非阻塞
    result = make_socket_nonblocking(listen_fd);
    if(result<0)
    {
        return -1;
    }

    int flag = 1;
    result = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof flag);
    assert(result >= 0);

    // 设置同一时间处于3次握手状态的最大连接数量
    result = listen(listen_fd, 128);
    assert(result >= 0);

    // 将监听套接字挂到epoll句柄指向的红黑树上
    event_set(&gevents[MAX_EVENTS], listen_fd, cb_accept, &gevents[MAX_EVENTS]);
    event_add(epfd, EPOLLIN|EPOLLET, &gevents[MAX_EVENTS]);

    return 1;
}


int make_socket_nonblocking(int sockfd)
{
    int flag;
    int result;

    flag = fcntl(sockfd, F_GETFL, 0);
    if(flag < 0)
    {
        return -1;
    }

    result = fcntl(sockfd, F_SETFL, flag|O_NONBLOCK);
    if(result<0)
    {
        return -1;
    }

    return 1;
}
