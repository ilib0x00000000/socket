/**
  * @author    ilib0x00000000
  * @time      2018/1/7
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/7.
//

// #include <stdio.h>
// #include <cxxabi.h>
#include <stdlib.h>
#include <execinfo.h>

#include "exception.h"


namespace muduo
{
    namespace detail
    {
        Exception::Exception(char *msg): message(msg)
        {
            fill_stack_trace();
        }

        Exception::Exception(std::string &msg): message(msg)
        {
            fill_stack_trace();
        }

        Exception::Exception(const Exception& ce)
        {
            ce.message = this->message;
            ce.stack = "";
        }

        Exception::~Exception() {}

        // 返回异常信息
        const char* Exception::what() const throw()
        {
            return message.c_str();
        }

        // 返回出现异常时的栈帧信息
        const char* Exception::stack_trace() const throw()
        {
            return stack.c_str();
        }

        // 设置异常信息 栈帧信息   这里返回的追溯信息是编译后的符号链接
        // 如果需要将符号链接转成可读的字符，需要使用  abi::__cxa_demangle
        void Exception::fill_stack_trace()
        {
            const int32_t len = 200;
            void* buffer[len];    // 最多保存200个栈帧的信息

            int32_t nptrs = ::backtrace(buffer, len);  // 最多追溯200层函数调用
            char** strings = ::backtrace_symbols(buffer, nptrs);  // 如果有返回信息，需要释放指针

            if(strings)
            {
                // 把全部异常的栈帧信息追加到string stack中
                for(int32_t i=0; i<nptrs; i++)
                {
                    stack.append(strings[i]);
                    stack.push_back('\n');
                }

                free(strings);
            }
        }
    }
}