/**
  * @author    ilib0x00000000
  * @time      2018/1/12
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/12.
//

#include "lock.h"
#include "condition.h"
#include "countdown.h"

namespace muduo
{
    CountDown::CountDown(int ct): mutex(), cond(mutex),counter(ct)
    {}

    void CountDown::wait()
    {
        MutexLockGuard guard(mutex);
        while(counter > 0)
        {
            cond.wait();
        }
    }

    void CountDown::down()
    {
        MutexLockGuard guard(mutex);

        --counter;    // 计数器-1

        if(counter == 0)
        {
            cond.notify_all();
        }
    }

    int CountDown::get_counter() const
    {
        MutexLockGuard guard(mutex);
        return counter;
    }
}