/**
  * @author    ilib0x00000000
  * @time      2018/1/10
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/10.
//
#ifndef DEVENT_ATOMICOPT_HPP
#define DEVENT_ATOMICOPT_HPP

/*
 * 原子操作API：
 *
 * 原子自增操作
 * type __sync_fetch_and__add(type *ptr, type value);
 *
 * 原子比较和交换操作
 * type __sync_val_compare_and_swap(type *ptr, type oldval, type newval);
 *                                              如果*ptr和oldval相等  赋值为newval
 * bool __sync_bool_compare_and_swap(type *ptr, type oldval, type newval);
 *
 * 原子赋值操作
 * type __sync_lock_test_and_set(type *ptr, type value);
 *
 *
 * 警告：
 *      这些API需要CPU和编译器同时支持
 *      编译时需要添加选项 -march=cpu-type   [native]
 *
 * 参考：
 *      https://github.com/ilib0x00000000/socket/blob/master/man/atomic.cpp
 */

#include <stdint.h>

#include "noncopyable.h"

namespace muduo
{
    namespace detail
    {
        template<typename T>
        class AtomicOptionT: public noncopyable
        {
        public:
            AtomicOptionT();
            ~AtomicOptionT();
            T get() ;                    // 返回当前值
            T add(const T& x);           // 原子性加上某一个值
            T sub(const T& x);           // 原子性减去某一个值
            T& increment();              // +1
            T& decrement();              // -1
            T set(const T& newval);      // 原子性赋值
            bool equal(T& x) const;      // 原子性比较
            bool operator==(T& x) const; // 原子性比较
        private:
            // function
        private:
            // data
            volatile T value;      // 防止编译器对代码进行优化    多线程常用
        };

        template<typename T>
        AtomicOptionT<T>::AtomicOptionT(): value(0)
        {}

        template<typename T>
        AtomicOptionT<T>::~AtomicOptionT()
        {}

        template<typename T>
        T  AtomicOptionT<T>::get()
        {
            return __sync_val_compare_and_swap(&value, 0, 0);
        }

        template <typename T>
        T AtomicOptionT<T>::add(const T& x)
        {
            return __sync_fetch_and_add(&value, x);
        }

        template <typename T>
        T AtomicOptionT<T>::sub(const T& x)
        {
            return __sync_fetch_and_add(0-x);
        }

        template <typename T>
        T& AtomicOptionT<T>::increment()
        {
            return __sync_fetch_and_add(&value, 1);
        }

        template <typename T>
        T& AtomicOptionT<T>::decrement()
        {
            return __sync_fetch_and_add(&value, -1);
        }

        template <typename T>
        T AtomicOptionT<T>::set(const T& newval)      // 原子性赋值
        {
            return __sync_lock_test_and_set(&value, newval);
        }

        template <typename T>
        bool AtomicOptionT<T>::equal(T& x) const      // 原子性比较
        {
            // 比较value和x的值是否相等
            // 如果相等返回true，并把x的值赋值给value
            return __sync_bool_compare_and_swap(&value, x, x);
        }

        template <typename T>
        bool AtomicOptionT<T>::operator==(T& x) const
        {
            return __sync_bool_compare_and_swap(&value, x, x);
        }
    }

    typedef detail::AtomicOptionT<int32_t> AtomicInt32;
    typedef detail::AtomicOptionT<int64_t> AtomicInt64;
}


#endif   // DEVENT_ATOMICOPT_HPP