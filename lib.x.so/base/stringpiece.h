/**
  * @author    ilib0x00000000
  * @time      2018/1/19
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2018/1/19.
//

#ifndef DEVENT_STRINGPIECE_H
#define DEVENT_STRINGPIECE_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <iostream>


namespace muduo
{
    class StringArg
    {
    public:
        StringArg(const char *str_): str(str_)
        {}

        StringArg(const std::string& str_): str(str_.c_str())
        {}

        const char* c_str() const
        {
            return str;
        }
    private:
        const char *str;
    };


    class StringPiece
    {
    public:
        StringPiece();
        StringPiece(const char *str);
        StringPiece(const unsigned char *str);
        StringPiece(const std::string& str);
        StringPiece(const char *offset, int len);

        const char* data() const;
        size_t size() const;
        bool empty() const;
        const char* begin() const;
        const char* end() const;

        void clear();
        void set(const char *str);
        void set(const char *str, int len);
        void set(const void *str, int len);
        void remove_prefix(int n);
        void remove_suffix(int n);

        char operator[](int i) const;
        bool operator==(const StringPiece& x) const;
        bool operator!=(const StringPiece& x) const;

        #define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp)                                                     \
            bool operator cmp (const StringPiece& x) const                                                    \
            {                                                                                                 \
                int32_t len = length < x.length ? length : x.length;                                          \
                int32_t r = memcmp(ptr, x.ptr, len);                                                          \
                return ( (r auxcmp 0) || ((r==0) && (length cmp x.length)) );                                 \
            }

            STRINGPIECE_BINARY_PREDICATE(< , <);
            STRINGPIECE_BINARY_PREDICATE(<=, <);
            STRINGPIECE_BINARY_PREDICATE(>=, >);
            STRINGPIECE_BINARY_PREDICATE(> , >);
        #undef STRINGPIECE_BINARY_PREDICATE

        int32_t compare(const StringPiece& x) const;
        std::string as_string() const;
        void copy_to_string(std::string *target) const;
        bool starts_with(const StringPiece& x) const;
    private:
        // fcuntion
    private:
        // data
        size_t   length;
        const char* ptr;
    };
}


#endif //DEVENT_STRINGPIECE_H
