/**
  * @author    ilib0x00000000
  * @time      2018/1/8
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/8.
//

#ifndef DEVENT_THREAD_H
#define DEVENT_THREAD_H

#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <functional>
#include <string>

#include "atomicopt.hpp"
#include "noncopyable.h"

namespace muduo
{
    class Thread: public noncopyable
    {
        /**
         * 需要重新设计
         *      当多个线程需要共享同一个对象时   比方  锁
         *      当线程去调用用户指定的任务时， 如何拿到参数
         *
         *  .start() -------> route()  ---------------> .run()
         *                     静态方法                   私有方法
         */
        // typedef void (*thread_func)(void) ;
        typedef std::function<void()> thread_func;
    public:
        explicit Thread(const thread_func&, const std::string& name=std::string());
        ~Thread();

        void  start();
        int   join();
        bool  is_started() const;
        bool  is_joined() const;
        bool  is_main() const;
        const std::string& get_thread_name() const;
    private:
        // function
        static void* route(void *arg)
        {
            Thread *th = static_cast<Thread*>(arg);
            th->run();
        }

        void run()
        {
            func();
        }
    private:
        // data
        bool started;
        bool joined;
        bool is_main_;
        pid_t pid;              // pid       系统级唯一标识
        pthread_t pthread_id;   // threadid  进程级唯一标识
        thread_func func;
        std::string name;
    };


    /**
     * 子类需要继承这个类，并实现handle方法
     */
    class ThreadBase: public noncopyable
    {
    public:
        ThreadBase(): pid(getpid()), tid(0), is_start(false), is_joined(false), is_main(false), name("")
        {}

        void start()
        {
            assert(!is_start);
            pthread_create(&tid, NULL, run, this);
            is_start = true;
            assert(tid > 0);
        }

        void join()
        {
            assert(is_start);
            assert(!is_joined);

            pthread_join(tid, NULL);
            is_joined = true;
        }
    private:
        // function
        virtual void handle()=0;

        static void* run(void *arg)
        {
            ThreadBase *tb = static_cast<ThreadBase*>(arg);
            tb->handle();
            return NULL;
        }
    private:
        // data
        pid_t pid;
        pthread_t tid;
        bool is_start;
        bool is_joined;
        bool is_main;
        std::string name;
    };
}


#endif //DEVENT_THREAD_H
