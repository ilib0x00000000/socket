/**
  * @author    ilib0x00000000
  * @time      2017/12/24
  * @email     ilib0x00000000@aliyun.com
*/

//
// Created by ilib0x00000000 on 2017/12/24.
//

#ifndef DEVENT_COPYABLE_H
#define DEVENT_COPYABLE_H

namespace muduo
{
    class copyable
    {
    public:
        copyable(){}
        copyable(const copyable& cp)
        {}
        ~copyable(){}
    };
}



#endif //DEVENT_COPYABLE_H
