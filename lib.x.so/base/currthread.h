/**
  * @author    ilib0x00000000
  * @time      2018/1/10
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/10.
//

#ifndef DEVENT_CURRENTTHREAD_H
#define DEVENT_CURRENTTHREAD_H

#include <unistd.h>
#include <pthread.h>

#include "detail.h"
#include "noncopyable.h"

//__thread int t_cached_tid;
//__thread const char *t_thread_name;

namespace muduo
{
    namespace currentthread
    {
         __thread int t_cached_tid = 0;
         __thread const char *t_thread_name = "unknown";
        // extern __thread int t_cached_tid;
        // extern __thread const char *t_thread_name;

        void cache() {
            if(t_cached_tid == 0) {
                t_cached_tid = detail::get_tid();
            }
        }

        int get_and_set_tid() {
            cache();
            if(t_cached_tid == getpid()) {
                t_thread_name = "main";
            }else {
                t_thread_name = "non-main";
            }

            return t_cached_tid;
        }

        inline void set_thread_name(char *name) {
            t_thread_name = name;
        }

        inline const char* get_thread_name() {
            return t_thread_name;
        }

        inline bool is_main() {
            return t_cached_tid == getpid();
        }
    }

    namespace detail
    {
        void after_fork()
        {
            currentthread::t_cached_tid  = 0;
            currentthread::t_thread_name = "main";

            currentthread::get_and_set_tid();
        }
    }


    // 在进程启动的时候自动调用
    class ThreadInitializer: public noncopyable
    {
    public:
        ThreadInitializer()
        {
            currentthread::get_and_set_tid();
            // 当fork之后  会自动调用注册的函数
            pthread_atfork(NULL, NULL, &(detail::after_fork));
        }
    };

    ThreadInitializer _init;   // 程序初始化时  调用   会设置 主线程的一些参数 同时注册一个函数
}

#endif //DEVENT_CURRENTTHREAD_H
