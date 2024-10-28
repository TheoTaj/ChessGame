#include<stdio.h>

void f1();
void f2();

int main ()
{
    printf("Appel de f1\n");
    f1();
    printf("Appel de f2\n");
    f2();
    printf("End\n");

    return 0;
}

void f1()
{
    int res = 0;
    for(int i = 0; i < 1000000; i++)
    {
        res += 1;
    }

    return ;
}

void f2()
{
    int res = 0;
    for(int i = 0; i < 100; i++)
    {
        res += 1;
    }

    return ;

}