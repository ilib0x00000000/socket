#ifndef _NONCOPYABLE_H_
#define _NONCOPYABLE_H_


/**
 * 不可拷贝的类
 *      构造与析构方法  实现     但是为空
 *
 *      拷贝构造和赋值操作符 私有的  且未实现
 */


class noncopyable
{
public:
    noncopyable()  {}

    virtual ~noncopyable() {}
private:
    noncopyable(const noncopyable& cn);
    const noncopyable& operator=(const noncopyable& cn);
};



#endif // _NONCOPYABLE_H_