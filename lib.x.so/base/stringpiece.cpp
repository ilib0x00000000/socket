/**
  * @author    ilib0x00000000
  * @time      2018/1/19
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/19.
//

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stringpiece.h"

namespace muduo
{

    StringPiece::StringPiece(): ptr(NULL), length(0)
    {}

    StringPiece::StringPiece(const char *str): ptr(str),
        length(static_cast<size_t>(strlen(str)))
    {}

    StringPiece::StringPiece(const unsigned char *str):
        ptr(reinterpret_cast<const char*>(str)), length(static_cast<size_t>(strlen(str)))
    {}

    StringPiece::StringPiece(const std::string& str):
        ptr(str.c_str()), length(static_cast<size_t>(str.size()))
    {}

    StringPiece::StringPiece(const char *offset, int len): ptr(offset), length(len)
    {}

    const char* StringPiece::data() const
    {
        return ptr;
    }

    size_t StringPiece::size() const
    {
        return length;
    }

    bool StringPiece::empty() const
    {
        return length == 0;
    }

    const char* StringPiece::begin() const
    {
        return ptr;
    }

    const char* StringPiece::end() const
    {
        return ptr + static_cast<int32_t>(length);
    }

    void StringPiece::clear()
    {
        ptr = NULL;
        length = 0;
    }

    void StringPiece::set(const char *str)
    {
        ptr = str;
        length = static_cast<size_t>(strlen(str));
    }

    void StringPiece::set(const char *str, int len)
    {
        ptr = str;
        length = static_cast<size_t>(len);
    }

    void StringPiece::set(const void *str, int len)
    {
        ptr = reinterpret_cast<const char*>(str);
        length = static_cast<size_t>(len);
    }

    // 抛弃前n个字符
    void StringPiece::remove_prefix(int n)
    {
        ptr += n;
        length -= n;
    }

    // 抛弃最后n个字符
    void StringPiece::remove_suffix(int n)
    {
        length -= n;
    }

    char StringPiece::operator[](int i) const
    {
        return ptr[i];
    }

    bool StringPiece::operator==(const StringPiece& x) const
    {
        return (length==x.length && memcmp(ptr, x.ptr, length));
    }

    bool StringPiece::operator!=(const StringPiece& x) const
    {
        return !(*this == x);
    }

    int32_t StringPiece::compare(const StringPiece& x) const
    {
        return 0;
    }

    std::string StringPiece::as_string() const
    {
        return std::string(ptr);
    }

    void StringPiece::copy_to_string(std::string *target) const
    {
        target->assign(ptr, length);
    }

    bool StringPiece::starts_with(const StringPiece& x) const
    {
        return (length>=x.length && memcmp(ptr, x.ptr, x.length));
    }
}

