/**
  * @author    ilib0x00000000
  * @time      2018/1/7
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/7.
//

#ifndef DEVENT_EXCEPTION_H
#define DEVENT_EXCEPTION_H

#include <string>
#include <exception>

/**
 * backtrace()     栈回溯，保存各个栈帧的地址
 * backtrace_symbols()   根据地址，转成相应的函数符号
 * abi::__cxa_demangle
 *
 * 以上函数实际是返回返回调用的层级信息，会从
 *              程序
 *              glibc
 *              main
 *              ...   逐级返回 直到调用这几个接口的函数
 */

namespace muduo
{
    namespace detail
    {
        class Exception: public std::exception
        {
        public:
            explicit Exception(char *msg);
            explicit Exception(std::string &msg);
            Exception(const Exception& ce);
            ~Exception();

            virtual const char* what() const throw();
            const char* stack_trace() const throw();
        private:
            // function
            void fill_stack_trace();
        private:
            // data
            std::string stack;
            std::string message;
        };
    }
}

#endif //DEVENT_EXCEPTION_H
