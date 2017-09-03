#include <sys/time.h>
#include <cstdio>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <errno.h>
#include <cstdlib>
#include <unistd.h>
#include <zconf.h>
#include <cctype>
#include <sys/epoll.h>

/**
 * @author:       ilib0x00000000
 * @email:        ilib0x00000000@aliyun.com
 * @date:         17-9-3
 * os:            ubuntu 16.04.4
 * c compiler:    gcc -v 5.4.0
 * c++ compiler:  g++ -v 5.4.0
 * @github:       https://github.com/ilib0x00000000
 */

/**
 * epoll api:
 *      int epoll_create(int size);
 *      int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
 *      int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
 *          timeout:
 *                  -1 阻塞
 *                  0  非阻塞
 *                  >0 毫秒
 *          return:
 *                  -1 失败
 *                  >0 就绪的文件描述符
 *
 */

#define MAXEVENTS 100
#define SERVER_PORT 6666
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE   4096

void main()
{
    int epfd;   // epoll文件句柄
    int nready;
    int result;
    int sock_fd;
    int listen_fd;
    char buff[BUFF_SIZE];
    struct epoll_event ev;
    struct epoll_event retevs[MAXEVENTS];
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof client_addr;

    // 创建一个epoll文件句柄
    epfd = epoll_create(100);
    assert(epfd >= 0);

    // 创建一个socket文件描述符
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    // 初始化地址
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //    result = inet_pton(AF_INET, SERVER_ADDR, reinterpret_cast<struct sockaddr *>(&server_addr));
    //    assert(result > 0);

    // 设置端口重用
    int flag = 1;
    result = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof flag);
    assert(result >= 0);

    // 绑定地址
    result = bind(listen_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof server_addr);
    assert(result >= 0);

    // 设置同一时间处于3次握手状态的最大连接数
    result = listen(listen_fd, 128);
    assert(result >= 0);

    ev.events = EPOLLIN;   // 读事件
    ev.data.fd = listen_fd;
    result = epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);
    assert(result >= 0);

    while(true)
    {
EPOLL_WAIT_AGAIN:
        // 阻塞式等待，直到epoll监听的文件描述符有就绪的
        nready = epoll_wait(epfd, retevs, MAXEVENTS, -1);

        if(nready < 0)
        {
            if(errno == EINTR)
            {
                goto EPOLL_WAIT_AGAIN;   // 被信号中断
            }else
            {
                perror("main:epoll_wait");
                exit(-1);
            }
        }

        for(int i=0; i<nready; i++)
        {
            if(retevs[i].data.fd == listen_fd)
            {
ACCEPT_AGAIN:
                sock_fd = accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);

                if(sock_fd < 0)
                {
                    if(errno==EINTR)
                    {
                        goto ACCEPT_AGAIN;
                    }else
                    {
                        perror("main:accept");
                        // log
                    }
                }else
                {
                    ev.events = EPOLLIN;
                    ev.data.fd = sock_fd;
                    result = epoll_ctl(epfd, EPOLLIN, sock_fd, &ev);
                    if(result < 0)
                    {
                        close(sock_fd);
                        // log
                    }
                }
            }else
            {
                /**
                 * 这里是单进程版，当有多个文件描述符就绪，可以读写时，只有一个文件描述符读写结束之后，其他文件描述符才能读写
                 */
                int conn_fd;  // 已经连接的文件描述符，可以读写
                conn_fd = retevs[i].data.fd;

                int n = read(conn_fd, buff, BUFF_SIZE-1);
                buff[n] = 0;

                while(n>0)
                {
                    for(int i=0; i<n; i++)
                    {
                        buff[i] = toupper(buff[i]);
                    }

                    int nw = write(conn_fd, buff, n);
                    assert(nw == n);

                    n = read(conn_fd, buff, BUFF_SIZE-1);
                    buff[n] = 0;
                }
EPOLL_CTL_DEL_AGAIN:
                ev.data.fd = conn_fd;
                result = epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, &ev);
                if(result < 0)
                    goto EPOLL_CTL_DEL_AGAIN;
                close(conn_fd);
            }
        }
    }
}
