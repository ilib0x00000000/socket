/**
  * @author    ilib0x00000000
  * @time      2018/1/12
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/12.
//

#include <errno.h>
#include <pthread.h>

#include "exception.h"
#include "condition.h"

namespace muduo
{
//    explicit Condition::Condition(MutexLock &mt):mutex(mt)
//    {
//        if(pthread_cond_init(&cond, NULL) != 0)
//        {
//            detail::Exception e("init condition failed");
//            throw e;
//        }
//    }

    Condition::~Condition()
    {
        if(pthread_cond_destroy(&cond) != 0)
        {
            detail::Exception e("destroy condition failed");
            throw e;
        }
    }

    void Condition::wait()
    {
        MutexLockGuard guard(mutex);   // 加锁
        if( pthread_cond_wait(&cond, mutex.get_mutex()) != 0)
        {
            detail::Exception e("pthread_conf_wait error");
            throw e;
        }
    }

    void Condition::notify()
    {
        MutexLockGuard guard(mutex);    // 加锁
        if( pthread_cond_signal(&cond) != 0)
        {
            detail::Exception e("condition notify error");
            throw e;
        }
    }

    void Condition::notify_all()
    {
        MutexLockGuard guard(mutex);
        if( pthread_cond_broadcast(&cond) != 0)
        {
            detail::Exception e("condition notify all error");
            throw e;
        }
    }

    bool Condition::time_wait(int64_t seconds)
    {
        const int64_t kNanoSecondsPerSeconds = 1e9;
        int64_t nano_seconds = seconds * kNanoSecondsPerSeconds;

        // 设置超时到时时间
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nano_seconds)/kNanoSecondsPerSeconds);
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec+nano_seconds) % kNanoSecondsPerSeconds);

        MutexLockGuard guard(mutex);
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get_mutex(), &abstime);
    }
}