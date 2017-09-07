#include <iostream>
#include <unistd.h>

#include "thrdpool.h"


void *task(void *arg)
{
    printf("thread 0x%lu working in task %d ====== ", pthread_self(), *(int *)arg);
    sleep(1);
    printf(" end .....\n");

    return NULL;
}


int main() {
    int i;
    int num[50];

    /**
     * 创建线程池
     *
     * 最少10个线程， 最多50个线程   最大80个任务
     */
    threadpool_t *pool = threadpool_create(10, 50, 80);

    if(pool)
    {
        std::cout << "创建线程池成功" << std::endl;
    }

    // 添加任务
    for(i=0; i<50; i++)
    {
        num[i] = i;
        add_task(pool, task, (void *)&num[i]);
    }

    sleep(10);


    // 销毁线程池
    threadpool_destory(pool);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}