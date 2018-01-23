/**
  * @author    ilib0x00000000
  * @time      2018/1/12
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/12.
//

#ifndef DEVENT_COUNTDOWN_H
#define DEVENT_COUNTDOWN_H

#include "lock.h"
#include "condition.h"
#include "noncopyable.h"

/**
 * 计数器
 */

namespace muduo
{
    class CountDown: public noncopyable
    {
    public:
        explicit CountDown(int ct);
        void wait();
        void down();
        // void p();
        // void v();
        int get_counter() const;
    private:
        // function
    private:
        // data
        mutable MutexLock mutex;   // 可变的
        Condition cond;  // 没有默认构造函数，必须显示初始化
        int counter;
    };
}

#endif //DEVENT_COUNTDOWN_H
