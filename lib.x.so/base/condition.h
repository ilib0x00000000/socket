/**
  * @author    ilib0x00000000
  * @time      2018/1/12
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/12.
//

#ifndef DEVENT_CONDITION_H
#define DEVENT_CONDITION_H

#include "lock.h"
#include "noncopyable.h"

#include <pthread.h>

namespace muduo
{
    class Condition: public noncopyable
    {
    public:
        explicit  Condition(MutexLock& mt): mutex(mt)
        {
            if(pthread_cond_init(&cond, NULL) != 0)
            {
                detail::Exception e("init condition failed");
                throw e;
            }
        }

        ~Condition();

        void wait();
        void notify();
        void notify_all();

        bool time_wait(int64_t seconds);
    private:
        // function
    private:
        // data
        MutexLock& mutex;
        pthread_cond_t cond;
    };
}




#endif //DEVENT_CONDITION_H
