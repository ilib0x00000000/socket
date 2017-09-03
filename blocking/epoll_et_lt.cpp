#include <unistd.h>
#include <sys/epoll.h>
#include <cassert>
#include <iostream>

/**
 * @author:       ilib0x00000000
 * @email:        ilib0x00000000@aliyun.com
 * @date:         17-9-3
 * @os:           ubuntu 16.04.4
 * @c compiler:   gcc -v 5.4.0
 * @c++ compiler: g++ -v 5.4.0
 * @github:       https://github.com/ilib0x00000000
 */

/**
 * epoll有两种模型： ET（边沿触发）和LT（水平触发）
 *
 * 默认水平触发，当文件描述符缓冲器只要还有数据，则epoll_wait()返回时，不停的提示就绪
 * 而ET（边沿触发），则不是。当缓冲区中有数据时，如果不是一次读完，在epoll_wait()返回时不会提示就绪，除非向文件描述符中写入新的数据
 */


void main()
{
    int pf[2];  // 管道返回的文件描述符
    pid_t pid;

    pipe(pf);    // 0读1写

    pid = fork();

    if(pid == 0)
    {
        // 子进程中   子进程向管道中写入数据
        close(pf[0]);

        int i;
        char ch = 'a';
        char buff[10];

        while (true)
        {
            // 子进程中，每隔5s向管道中写入10个字符
            for(i=0; i<5; i++)
            {
                buff[i] = ch;
            }
            write(pf[1], buff, 5);
            ch ++;
            buff[4] = '\n';
            /***************************************10个字符分为2行*****************************************************/
            for(; i<10; i++)
            {
                buff[i] = ch;
            }
            write(pf[1], buff+5, 5);
            ch ++;
            buff[9] = '\n';

            sleep(5);
            std::cout << "----------------过去5秒----------------" << std::endl;
        }
    }else
    {
        // 父进程     父进程从管道中读取数据
        int epfd;
        int nready;
        int result;
        char buff[6];
        struct epoll_event ev;
        struct epoll_event retevts[5];

        close(pf[1]);

        epfd = epoll_create1(0);   // 创建一个epoll句柄
        assert(epfd>=0);

//        ev.events = EPOLLIN;   // 水平触发   默认
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = pf[0];

        result = epoll_ctl(epfd, EPOLL_CTL_ADD, pf[0], &ev);
        assert(result >= 0);

        while(true)
        {
            nready = epoll_wait(epfd, retevts, 5, -1);    // 阻塞式调用epoll_wait
            assert(nready >= 0);

            // 一次读取5个字符
            int n = read(retevts[0].data.fd, buff, 5);
            buff[5] = 0;
            std::cout << buff << std::endl;
        }
    }
}
/**
ET 边沿触发输出：
    解释：子进程每次写入5个字符，连续写入2次，所以缓冲区里面有10个字符（也可能中间写入的时候，CPU时间片被抢占，先不考虑），使用ET模型时，每次只读取5个字符，
    这就导致了缓冲区里面还剩5个字符，然后再次进入epoll_wait，但是ET此时并不会返回，只有等到子进程再次向管道（缓冲区）中写入10个字符，此时管道（缓冲区）中
    共有15个字符，然后epoll_wait返回，再次从管道（缓冲区）中读取5个字符。。。。。。。
aaaaa
----------------过去5秒----------------
bbbbb
----------------过去5秒----------------
ccccc
----------------过去5秒----------------
ddddd
----------------过去5秒----------------
eeeee
*/

/**
LT 水平触发输出:
    解释：子进程向管道中写入10个字符之后睡眠，父进程有足够的CPU时间片，这时父进程先从管道（缓冲区）中读取5个字符，然后进入epoll_wait，但是随即就返回了，
    然后再从管道（缓冲区）中读取5个字符，之后进入epoll_wait，这时管道（缓冲区）中没有数据，epoll_wait被阻塞，直到子进程向管道（缓冲区）中写入数据。。。
aaaaa
bbbbb
----------------过去5秒----------------
ccccc
ddddd
----------------过去5秒----------------
eeeee
fffff
----------------过去5秒----------------
ggggg
hhhhh
----------------过去5秒----------------
iiiii
jjjjj
*/
