#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include "thrdpool.h"


#define DEFAULT_TIME        10    // 10s检测一次
#define MIN_WAIT_TASK_NUM   10    // 如果queue_size>MIN_WAIT_TASK_NUM，添加新的线程到线程池
#define DEFAULT_THREAD_VARY 10    // 每次创建和销毁线程的个数


// 创建线程池
threadpool_t *threadpool_create(int min_thrd_num, int max_thrd_num, int queue_max_size)
{
    int i;

    // 创建一个指针指向线程池对象
    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    if(!pool)
    {
        return NULL;
    }

    // 初始化互斥量
    if(pthread_mutex_init(&(pool->lock), NULL)!=0 || pthread_mutex_init(&(pool->thread_counter), NULL)!=0)
    {
        return NULL;
    }

    // 初始化条件变量
    if(pthread_cond_init(&(pool->queue_not_empty), NULL)!=0 || pthread_cond_init(&(pool->queue_not_full), NULL)!=0)
    {
        return NULL;
    }

    // 线程数组
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t)*max_thrd_num);
    if(!(pool->threads))
    {
        free(pool);
        pool = NULL;
        return NULL;
    }

    // 任务队列
    pool->task_queue = (task_t *)malloc(sizeof(task_t)*queue_max_size);
    if(!(pool->task_queue))
    {
        free(pool->threads);
        pool->threads = NULL;

        free(pool);
        pool = NULL;
        return NULL;
    }

    pool->min_thrd_num = min_thrd_num;  // 线程池中最少的线程数
    pool->max_thrd_num = max_thrd_num;  // 线程池中最大线程数
    pool->live_thrd_num = min_thrd_num; // 线程池中当前存活的线程数
    pool->busy_thrd_num = 0;            // 线程池中正在工作的线程数
    pool->wait_exit_thrd_num = 0;       // 等待清理的线程数目


    pool->queue_front = pool->queue_back = pool->queue_size = 0;
    pool->queue_max_size = queue_max_size;

    pool->shutdown = false;

    // 创建线程
    for(i=0; i<min_thrd_num; i++)
    {
        pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
    }
    pthread_create(&(pool->manager_thrd), NULL, adjust_thread, (void *)pool);

    return pool;
}


// 线程池中各个工作线程   如果队列不为空， 就要有线程去处理任务
void *threadpool_thread(void *threadpool)
{
    task_t task;
    threadpool_t *pool = (threadpool_t *)threadpool;

    while(true)
    {
        pthread_mutex_lock(&(pool->lock));  // 加锁

        // 任务队列为空 且线程池没有关闭
        while(pool->queue_size==0 && (!pool->shutdown))
        {
            // 任务队列为空，则阻塞， 等待被唤醒
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));

            // 清除指定数目的空闲线程 如果要结束的线程个数大于0 结束当前线程
            if(pool->wait_exit_thrd_num > 0)
            {
                pool->wait_exit_thrd_num--;

                // 如果当前线程池中存活的线程的个数大于最小线程数目  则结束当前线程
                if(pool->live_thrd_num > pool->min_thrd_num)
                {
                    pool->live_thrd_num--;
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(NULL);
                }
            }
        }

        // 如果关闭了线程池，结束每个线程
        if(pool->shutdown)
        {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        // 将任务队列队首指针指向的任务赋值给局部变量
        task.callback = pool->task_queue[pool->queue_front].callback;
        task.arg = pool->task_queue[pool->queue_front].arg;

        // 队列指针向后移动1位   模拟环形队列
        pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
        pool->queue_size--;

        // 通知可以有新的任务添加进来
        pthread_cond_broadcast(&(pool->queue_not_full));

        // 任务取出来之后  立即将线程池 锁释放
        pthread_mutex_unlock(&(pool->lock));


        // 给工作线程数目+1
        pthread_mutex_lock(&(pool->lock));
        pool->busy_thrd_num ++;
        pthread_mutex_unlock(&(pool->lock));


        /**
         * 执行任务
         */
        task.callback(task.arg);


        // 任务执行完毕  工作线程数目-1
        pthread_mutex_lock(&(pool->lock));
        pool->busy_thrd_num --;
        pthread_mutex_unlock(&(pool->lock));
    }

    pthread_exit(NULL);
    return NULL;
}


// 管理线程
void *adjust_thread(void *threadpool)
{
    int i;
    int queue_size;    // 任务数目
    int live_thrd_num; // 生存的线程数目
    int busy_thrd_num; // 工作线程数目
    threadpool_t *pool = (threadpool_t *)threadpool;

    while(!pool->shutdown)
    {
        sleep(DEFAULT_TIME);   // 睡眠10s  定时管理

        pthread_mutex_lock(&(pool->lock));
        queue_size = pool->queue_size;
        live_thrd_num = pool->live_thrd_num;
        busy_thrd_num = pool->busy_thrd_num;
        pthread_mutex_unlock(&(pool->lock));

        /*
         * 新增线程
         *
         * 算法：任务数大于最小线程数目， 且存活的线程数小于最大线程数目
         */
        if(queue_size >= MIN_WAIT_TASK_NUM && live_thrd_num < pool->max_thrd_num)
        {
            // 一次添加10个线程
            int add=0;

            pthread_mutex_lock(&(pool->lock));

            for(i=0; i<pool->max_thrd_num && add<DEFAULT_THREAD_VARY && pool->live_thrd_num<pool->max_thrd_num; add++)
            {
                // 检查线程池指向的线程数组中对应的位置线程是否存活
                if(pool->threads[i]==0 || !is_thread_alive(pool->threads[i]))
                {
                    pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
                    add++;
                    pool->live_thrd_num++;
                }
            }

            pthread_mutex_unlock(&(pool->lock));
        }


        /*
         * 销毁线程
         *
         * 算法： 2*工作线程 < 存活线程   且  存活的线程 > 最小线程数目
         */
        if((busy_thrd_num*2)<live_thrd_num && live_thrd_num>pool->min_thrd_num)
        {
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thrd_num = DEFAULT_THREAD_VARY;   // 设置需要销毁的线程数目为10
            pthread_mutex_unlock(&(pool->lock));


            for(i=0; i<DEFAULT_THREAD_VARY; i++)
            {
                pthread_cond_signal(&(pool->queue_not_empty));  // 这是会有线程收到，恢复执行，然后检验是否需要销毁线程，需要的话会自动结束
            }
        }
    }

    return NULL;
}


// 向线程池中添加一个任务
int add_task(threadpool_t *pool, void*(*function)(void *arg), void *arg)
{
    pthread_mutex_lock(&(pool->lock));


    // 队列如果已满 则阻塞
    while((pool->queue_size==pool->queue_max_size) && (!pool->shutdown))
    {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
    }

    if(pool->shutdown)
    {
        pthread_mutex_unlock(&(pool->lock));
    }

    // 清空原来任务的参数
    if(pool->task_queue[pool->queue_back].arg != NULL)
    {
        free(pool->task_queue[pool->queue_back].arg);
        pool->task_queue[pool->queue_back].arg = NULL;
    }

    // 添加任务到任务队列中
    pool->task_queue[pool->queue_back].callback = function;
    pool->task_queue[pool->queue_back].arg = arg;
    pool->queue_back = (pool->queue_back+1) % pool->queue_max_size;
    pool->queue_size ++;

    // 添加任务之后，任务队列不为空，唤醒线程池中的等待线程
    pthread_cond_signal(&(pool->queue_not_empty));

    pthread_mutex_unlock(&(pool->lock));

    return 0;
}



// 测试线程是否存活
int is_thread_alive(pthread_t tid)
{
    int kill_rc;

    kill_rc = pthread_kill(tid, 0);   // 发信号0， 测试线程是否存活

    if(kill_rc == ESRCH)
    {
        return false;
    }

    return true;
}

// 销毁线程池
int threadpool_destory(threadpool_t *pool)
{
    int i = 0;
    if(pool == NULL)
    {
        return -1;
    }
    pool->shutdown = true;

    // 等待管理者线程退出
    pthread_join(pool->manager_thrd, NULL);

    for(i=0; i<pool->live_thrd_num; i++)
    {
        // 通知线程组中的线程
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }

    for(i=0; i<pool->live_thrd_num; i++)
    {
        // 等待所有线程结束
        pthread_join(pool->threads[i], NULL);
    }

    threadpool_free(pool);

    return 0;
}


// 释放线程池
int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL)
    {
        return -1;
    }

    if(pool->task_queue)
    {
        free(pool->task_queue);
    }

    if(pool->threads)
    {
        free(pool->threads);

        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));    // 销毁锁

        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter)); // 销毁锁

        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }

    free(pool);
    pool = NULL;

    return 0;
}



// 获取线程池存活线程数目
int threadpool_all_threadnum(threadpool_t *pool)
{
    int all_threadnum = -1;

    pthread_mutex_lock(&(pool->lock));
    all_threadnum = pool->live_thrd_num;
    pthread_mutex_unlock(&(pool->lock));


    return all_threadnum;
}


// 获取当前工作线程数目
int threadpool_busy_threadnum(threadpool_t *pool)
{
    int busy_threadnum = -1;

    pthread_mutex_lock(&(pool->thread_counter));
    busy_threadnum = pool->busy_thrd_num;
    pthread_mutex_unlock(&(pool->thread_counter));

    return busy_threadnum;
}
