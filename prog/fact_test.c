#include "stdio.h"

int fact(int n)
{
    int f=1;
    int i;
    for(i=1;i<=n;i++) {
        f *= i;
    }
    return f;
}

void fact_test(void)
{
    int i;
    for(i=1;i<=10;i++) {
        printf("%2d! = %d\n", i, fact(i));
    }
}
