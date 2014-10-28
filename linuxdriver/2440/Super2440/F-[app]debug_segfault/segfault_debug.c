
#include <stdio.h>

void C(int *p)
{
    *p = 0x28;
}

void B(int *p)
{
    C(p);
}

int A(int *p)
{
    B(p);
    return 0;
}

int A2(int *p)
{
    C(p);
    return 0;
}

int main(int argc, char **argv)
{
    int a;
    int *p = NULL;

    A2(&a);  /* A2 invoking C */
    printf("a = 0x%x\n", a);

    A(p);   /* A invoking B invoking C */
    
    return 0;
}


