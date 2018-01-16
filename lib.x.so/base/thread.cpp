/**
  * @author    ilib0x00000000
  * @time      2018/1/8
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/8.
//
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <string>

#include "thread.h"
#include "detail.h"
#include "currthread.h"

namespace muduo
{
    Thread::Thread(const thread_func &tf, const std::string &n)
            :func(tf), started(false), joined(false), pthread_id(0), name(n)
    {
        pid = getpid();
    }

    Thread::~Thread() {
        if(started && !joined) {
            pthread_detach(pthread_id);     // 如果线程还在运行且没有join，则分离该线程
        }
    }

    // 创建线程并启动
    void Thread::start() {
        assert(!started);  // 线程还没有运行   只有DEBUG版本下 assert()有效
        started = true;

        if(pthread_create(&pthread_id, NULL, route, this)) {
            started = false;
            // log
        }else {
            assert(pthread_id > 0);
        }
    }

    int Thread::join() {
        assert(started);
        assert(!joined);

        joined = true;
        pthread_join(pthread_id, NULL);
    }

    bool Thread::is_started() const {
        return started;
    }

    bool Thread::is_joined() const {
        return joined;
    }

    const std::string& Thread::get_thread_name() const {
        return name;
    }

    bool Thread::is_main() const {
        return is_main_;
    }
}