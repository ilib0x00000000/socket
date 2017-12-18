/**
  * @author    ilib0x00000000
  * @time      2017/12/18
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2017/12/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>

void sig_handle(int signo)
{
    printf("收到SIGPIPIE信号\n");
}


int main()
{
    int listfd;
    int sockfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    signal(SIGPIPE, sig_handle);
    // signal(SIGPIPE, SIG_IGN);     一般这里会忽略SIGPIPE信号

    listfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    assert(listfd >= 0);

    memset(&server_addr, sizeof(server_addr), 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(64423);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listfd, 10);

    sockfd = accept(listfd, NULL, 0);
    assert(sockfd >= 0);

    char buff[4096] = {0};
    int nread;

    nread = read(sockfd, buff, 4096);
    printf("****%s*****\n", buff);
    sleep(10);    // 等待客户端关闭
    
    /**
     * 如果客户端关闭套接字close()
     * 服务器再调用一次write，服务器会接收一个RST segment
     * 如果服务器接着还调用一次write，这个时候就会产生SIGPIPE信号
     */
    char *sbuf = "hello world";
    
    printf("第一次写开始\n");
    write(sockfd, (void *)sbuf, sizeof(sbuf));
    printf("第一次写结束\n");

    sleep(1);
    printf("第2次写开始\n");
    write(sockfd, (void *)sbuf, sizeof(sbuf));
    printf("第2次写结束\n");
    
    while(true)
    {
        ;
    }

    close(listfd);
    close(sockfd);

    return 0;
}
