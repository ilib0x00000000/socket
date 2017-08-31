#include <iostream>

/**
 * 多进程版socket服务器
 *
 * 1. 父进程负责accept
 * 2. 父进程调用信号处理函数SIGCHLD，并在信号处理函数中调用waitpid()
 * 3. 子进程负责处理socket数据的传输
 */
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#include "easylog.h"


#define SERVER_PORT 6666
#define BUFF_SIZE   4096

// 处理子进程终止时，内核发出的信号
void handle_zombie(int signo)
{
    while (waitpid(0, NULL, WNOHANG) > 0)
    {
        ;
    }

    return;
}


int main() {
    int result;
    int sock_fd;
    int listen_fd;
    struct sockaddr_in server_addr;

    EasyLog log_access("access.log");
    EasyLog log_error("error.log");

    // 僵尸进程处理
    signal(SIGCHLD, handle_zombie);


    // 创建一个socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    // 初始化地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // int ret = inet_pton(AF_INET, "192.168.1.174", &server_addr.sin_addr.s_addr);
    // 返回值： 1表示成功
    //         0表示字符串格式不对
    //        -1表示失败

    // 绑定主机地址
    result = bind(listen_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr));
    assert(result>=0);

    // 设置REUSE标志位
    int flag = 1;
    result = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    assert(result >= 0);


    // 指定同一时间处于3次握手的连接数
    result = listen(listen_fd, 128);
    assert(result >= 0);


    /**
     * 使用多进程处理连接
     */
    while(true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
ACCEPT_AGAIN:
        sock_fd = accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);
        if(sock_fd == -1)
        {
            // accept()是慢系统调用，有可能被信号中断，需要对errno做检查
            if(errno == EINTR || errno == EAGAIN)
            {
                goto ACCEPT_AGAIN;
            } else
            {
                perror("accept");
                // 将错误信息写入日志中
                char errmsg[256];
                sprintf(errmsg, "error:[%s:%s] %s", __FILE__, __LINE__, strerror(errno));
                log_error.log(errmsg);
            }
        } else
        {
            // 创建一个新的进程去处理这个连接
            pid_t subpid;
            subpid = fork();

            if(subpid < 0)
            {
                // 创建进程失败，将错误信息写入日志
                perror("fork");
                char errmsg[256];
                sprintf(errmsg, "error:[%s:%s] %s", __FILE__, __LINE__, strerror(errno));
                log_error.log(errmsg);
                close(sock_fd);
            } else
            if(subpid == 0)
            {
                // 这是子进程
                close(listen_fd);   // 关闭监听套接字
                // TODO;
                // read/write
                ssize_t n;
                char buff[BUFF_SIZE];

READ_AGAIN:
                // read也是慢系统调用，返回-1时，需要对errno做检测
                n = read(sock_fd, buff, BUFF_SIZE-1);
                if(n<0)
                {
                    if(errno==EINTR || errno==EAGAIN || errno==EWOULDBLOCK)
                    {
                        // errno==EINTR  表示被信号中断
                        // errno==EAGAIN/EWOULDBLOCK 表示非阻塞读取数据，但是数据没有准备好
                        goto READ_AGAIN;
                    } else
                    {
                        perror("read");
                        // 将错误信息写入日志
                        char errmsg[256];
                        sprintf(errmsg, "error:[%s:%s] %s", __FILE__, __LINE__, strerror(errno));
                        log_error.log(errmsg);
                        close(sock_fd);
                        exit(-1);
                    }
                } else
                if(n == 0)
                {
                    // 数据读取完毕
                    close(sock_fd);
                    exit(1);
                } else
                {
                    // 回写
                    // 写入access日志
                    buff[n] = '\0';
                    char logbuff[256];
                    sprintf(logbuff, "recvive %s: %s", inet_ntoa(client_addr.sin_addr), buff);
                    log_access.log(logbuff);

                    for(int i=0; i<n; i++)
                    {
                        buff[i] = toupper(buff[i]);
                    }
                    int wn = write(sock_fd, buff, n);
                    assert(wn == n);
                    goto READ_AGAIN;
                }
            } else
            {
                // 在父进程中
                close(sock_fd);     // 关闭客户端连接上的套接字
            }
        }
    }

    // 关闭套接字
    close(listen_fd);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}