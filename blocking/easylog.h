//
// Created by ilib0x00000000 on 17-8-30.
//

#ifndef MULTIPROCESSSOCKET_EASYLOG_H
#define MULTIPROCESSSOCKET_EASYLOG_H

/**
 * @author:       ilib0x00000000
 * @email:        ilib0x00000000@aliyun.com
 * @date:         17-8-30
 * os:            ubuntu 16.04.4
 * c compiler:    gcc -v 5.4.0
 * c++ compiler:  g++ -v 5.4.0
 * @github:       https://github.com/ilib0x00000000
 *
 * 自定义一个日志输出类
 */

#include <ctime>
#include <memory>
#include <fstream>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>


class EasyLog
{
public:
    explicit EasyLog(std::string filename);    // 构造函数
    explicit EasyLog(const char *filename);
    ~EasyLog();   // 析构函数
    void log(std::string msg) const;
    void log(char *msg) const;
private:
    int fd_;   // 文件描述符
};

void EasyLog::log(std::string msg) const
{
    char time_buff[64];
    time_t now = time(0);
    tm* now_format = localtime(&now);

    strftime(time_buff, 63, "%Y-%m-%d %H:%M:%S\t", now_format);

    write(this->fd_, time_buff, strlen(time_buff));
    write(this->fd_, msg.c_str(), msg.length());
    write(this->fd_, "\n", 1);
}

void EasyLog::log(char *msg) const
{
    char time_buff[64];
    time_t now = time(0);
    tm* now_format = localtime(&now);

    strftime(time_buff, 63, "%Y-%m-%d %H:%M:%S\t", now_format);

    write(this->fd_, time_buff, strlen(time_buff));
    write(this->fd_, msg, strlen(msg));
    write(this->fd_, "\n", 1);
}


EasyLog::EasyLog(std::string filename)
{
    this->fd_ = open(filename.c_str(), O_RDWR|O_APPEND|O_CREAT);
    if(this->fd_ < 0)
    {
        perror(strerror(errno));
        exit(-1);
    }
}

EasyLog::EasyLog(const char *filename)
{
    this->fd_ = open(filename, O_RDWR|O_APPEND|O_CREAT);
    if(this->fd_ < 0)
    {
        perror(strerror(errno));
        exit(-1);
    }
}

EasyLog::~EasyLog()
{
    close(this->fd_);
}

#endif //MULTIPROCESSSOCKET_EASYLOG_H
