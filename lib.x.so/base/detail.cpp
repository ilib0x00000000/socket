/**
  * @author    ilib0x00000000
  * @time      2017/12/15
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2017/12/15.
//

#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>

#include "detail.h"
#include "timestamp.h"
#include "timezone.h"

namespace muduo
{
    namespace detail
    {
        pid_t get_tid()
        {
            return syscall(SYS_gettid);
        }

        /*******************File类*************************/
        File::File(const char *file) :fp(fopen(file, "rb"))
        {}

        File::~File()
        {
            if(fp)
            {
                fclose(fp);
            }
        }

        bool File::is_valid() const
        {
            return fp;
        }

        std::string File::read_bytes(int n)
        {
            char buff[n];
            ssize_t nr = ::fread(buff, 1, n, fp);
            if(nr != n)
            {
                fprintf(stderr, "%s: line %d * read error\n", __FILE__, __LINE__);
                return std::string("");
            }
            return std::string(buff, n);
        }

        int32_t File::read_int32()
        {
            int32_t x = 0;
            ssize_t nr = ::fread(&x, 1, sizeof(int32_t),fp);
            if(nr != sizeof(int32_t))
            {
                fprintf(stderr, "%s: line %d * read error\n", __FILE__, __LINE__);
                return -1;
            }
            return x;
        }

        uint8_t File::read_uint8()
        {
            uint8_t x = 0;
            ssize_t nr = ::fread(&x, 1, sizeof(uint8_t), fp);
            if(nr != sizeof(uint8_t))
            {
                fprintf(stderr, "%s: line %d * read error\n", __FILE__, __LINE__);
                return -1;
            }
            return x;
        }
        /*******************End  class File*************************/

        bool read_time_zone_file(const char *zonefile, MData& data)
        {
            File f(zonefile);
            if(f.is_valid())
            {
                std::string head = f.read_bytes(4);
                if(head != "TZif")
                {
                    fprintf(stderr, "%s: line %d ** zone file error\n", __FILE__, __LINE__);
                    return NULL;
                }

                std::string version = f.read_bytes(1);
                f.read_bytes(15);

                int32_t is_gmt_cnt = f.read_int32();
                int32_t is_std_cnt = f.read_int32();
                int32_t leap_cnt = f.read_int32();
                int32_t time_cnt = f.read_int32();
                int32_t type_cnt = f.read_int32();
                int32_t char_cnt = f.read_int32();

                std::vector<int32_t> trans;
                std::vector<int32_t> localtimes;
                trans.reserve(time_cnt);

                for(int i=0; i<time_cnt; i++)
                {
                    trans.push_back(f.read_int32());
                }
                for(int i=0; i<time_cnt; i++)
                {
                    uint8_t local = f.read_uint8();
                    localtimes.push_back(static_cast<int32_t>(local));
                }
                for(int i=0; i<type_cnt; i++)
                {
                    int32_t gmt_off = f.read_int32();
                    uint8_t is_dst = f.read_uint8();
                    uint8_t abbrind = f.read_uint8();

                    data.localtimes.push_back(Localtime(gmt_off, is_dst, abbrind));
                }
            }
            return false;
        }

        const Localtime* find_local_time(const MData& data, Transition sentry, Comp comp)
        {
            return NULL;
        }
    }
}
