/**
os:            ubuntu 16.04.4
c compiler:    gcc -v 5.4.0
c++ compiler:  g++ -v 5.4.0
*/

#include <iostream>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

/**
 * api:
 *
 * address:
 *      htons()/ntohs()
 *      htonl()/ntohl()
 *      inet_pton()/inet_ntop()
 */

#define SERVER_PORT  6666
#define BUFF_SIZE    10
#define DEBUG

int main() {
    int n;
    int result;
    int listen_fd;
    int socket_fd;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);    // 创建一个socket
    assert(listen_fd > 0);


    // 初始化IP地址
    ::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定主机地址
    result = bind(listen_fd, reinterpret_cast<struct sockaddr *>(&server_addr), server_addr_len);
    assert(result >= 0);

    // 设置SO_REUSEADDR标记位
    int flag = 1;
    result = ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    assert(result >= 0);

    // 指定同一时间处于3次握手阶段的连接数的个数，超过这个数字之后，新的连接将等待，直到3次握手队列中有连接完成
    result = listen(listen_fd, 128);
    assert(result >= 0);

    while(true)
    {
#ifdef DEBUG
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        socket_fd = ::accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);

        n = ::read(socket_fd, buff, BUFF_SIZE-1);
        buff[n] = '\0';
        while(n>0)
        {
            std::cout << buff << std::endl;
            for(int i=0; i<n; i++)
            {
                buff[i] = toupper(buff[i]);
            }
            buff[n] = '\0';
            int size = ::write(socket_fd, buff, n);
            assert(n==size);
            memset(buff, 0, BUFF_SIZE);
            n = read(socket_fd, buff, BUFF_SIZE-1);
        }

        close(socket_fd);
#else
        socket_fd = ::accept(listen_fd, NULL, NULL);

        n = ::read(socket_fd, buff, BUFF_SIZE-1);
        buff[n] = 0;
        while(n>0)
        {
            cout << buff << endl;
            n = read(socket_fd, buff, BUFF_SIZE-1);
            buff[n] = 0;
        }

        close(socket_fd);
#endif
    }

    close(listen_fd);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
