#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * type __sync_fetch_and_add(type *ptr, type value);      原子自增操作
 * type __sync_lock_test_and_set(type *ptr, type value);  原子赋值操作
 *
 * type __sync_val_compare_and_swap(type *ptr, type oldval, type newval);  // 如果第一个指针指向的值与第二个参数相等， 则把指针指向的值赋值为第三个参数，并返回旧值
 * bool __sync_bool_compare_and_swap(type *ptr, type oldval, type newval); // 如果第一个指针指向的值与第二个参数相等，就返回True，并把指针指向的值赋值为第三个参数
 */


int main()
{
    int32_t a = 0;
    int32_t b = 0;
    int32_t ret;

    __sync_fetch_and_add(&a, 1);
    printf("%d\n", a);

    __sync_lock_test_and_set(&a, 10);
    printf("%d\n", a);

    printf("a=10\nb=0\n");

    ret = __sync_val_compare_and_swap(&a, 20, 20);
    printf("ret = %d\n", ret);
    printf("a = %d\n", a);

    ret = __sync_val_compare_and_swap(&a, 10, 20);        // 如果第一个指针指向的值与第二个参数相等， 则把指针指向的值赋值为第三个参数，并返回旧值
    printf("ret = %d\n", ret);
    printf("a = %d\n", a);

    ret = __sync_bool_compare_and_swap(&a, 20, 8);        // 如果第一个指针指向的值与第二个参数相等，就返回True，并把指针指向的值赋值为第三个参数
    printf("ret = %d\n", ret);
    printf("a = %d\n", a);

    return 0;
}
