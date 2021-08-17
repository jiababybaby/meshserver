#include "stdio.h"
typedef struct test1{
    char *p1;
    char *p2;
} test1;
int main()
{
    test1 test1_v;
    test1_v.p1 = "123";
    test1_v.p2 = "456";
    char *tmp[2] = {0};
    *tmp[0] = 0;
    printf("1: %x\n2: %x\n3： %x\n", &(test1_v.p1), &(test1_v.p2), &(test1_v.p1) + 1);
    char **p = (void *)(&(test1_v.p1) + 1);
    printf("%s\n", *p);
    printf("sizeof(char *): %d\n", sizeof(void *));
}