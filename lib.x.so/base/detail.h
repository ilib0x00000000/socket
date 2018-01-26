/**
  * @author    ilib0x00000000
  * @time      2017/12/15
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2017/12/15.
//

#ifndef DEVENT_DETAIL_H
#define DEVENT_DETAIL_H

#include <unistd.h>
#include <stdint.h>
#include <iostream>

#include "noncopyable.h"
#include "timezone.h"

namespace muduo
{
    namespace detail
    {
        pid_t get_tid();

        class File: public noncopyable
        {
        public:
            File(const char *file);
            ~File();
            bool is_valid() const;
            std::string read_bytes(int n);
            int32_t read_int32();
            uint8_t  read_uint8();
        private:
            // function
        private:
            // data
            FILE *fp;
        };

        bool read_time_zone_file(const char *zonefile, MData *data);
        const Localtime* find_local_time(const MData& data, Transition sentry, Comp comp);
    }
}

#endif //DEVENT_DETAIL_H
