
#ifndef THREAD_POOL
#define THREAD_POOL


#include <sys/param.h>

/**
 * 实现线程池
 *      1. 线程池
 *      2. 管理线程
 *      3. 任务队列
 */

// 任务信息
typedef struct threadpool_task_t{
    void *arg;
    void *(*callback)(void *arg);
}task_t;


// 线程池信息
struct threadpool_t
{
    // 互斥量
    pthread_mutex_t lock;             // 给线程池加的锁
    pthread_mutex_t thread_counter;   // 计算工作线程时加的锁

    // 条件变量
    pthread_cond_t  queue_not_full;   // 如果当前任务队列不为满，将唤醒向队列中添加任务的线程
    pthread_cond_t  queue_not_empty;  // 如果当前任务队列不为空，将唤醒线程池中的线程

    pthread_t *threads;             // 线程数组
    pthread_t manager_thrd;         // 管理线程
    task_t *task_queue;             // 任务队列

    /**
     * 线程池相关信息
     */
    int min_thrd_num;               // 线程池中最少的线程数
    int max_thrd_num;               // 线程池中最大线程数
    int live_thrd_num;              // 线程池中当前存活的线程数
    int busy_thrd_num;              // 线程池中正在工作的线程数
    int wait_exit_thrd_num;         // 等待清理的线程数目

    /**
     * 任务队列相关信息
     */
    int queue_front;                // 任务队列队首指针
    int queue_back;                 // 任务队列队尾指针
    int queue_size;                 // 任务队列的当前任务个数
    int queue_max_size;             // 任务队列可以容纳的最多任务数

    bool shutdown;
};


typedef struct threadpool_t threadpool_t;


threadpool_t *threadpool_create(int min_thrd_num, int max_thrd_num, int queue_size);
int add_task(threadpool_t *pool, void *(*callback)(void *arg), void *arg);
int threadpool_destory(threadpool_t *pool);


void *threadpool_thread(void *threadpool);
void *adjust_thread(void *threadpool);
int is_thread_alive(pthread_t tid);
int threadpool_free(threadpool_t *pool);


int threadpool_all_threadnum(threadpool_t *pool);
int threadpool_busy_threadnum(threadpool_t *pool);



#endif // THREAD_POOL
