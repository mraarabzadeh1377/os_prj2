#include "types.h"
#include "user.h"
#define NUMBER_OF_CHILD 10

int shifter(int *num)
{
    int rest = *num % 2;
    *num = (*num - *num % 2) / 2;
    return rest;
}
int determine_siz(int *num)
{
    int count = 1;
    int position = 0;
    while (count < *num)
    {
        position++;
        count *= 2;
    }
    *num -= count;
    return position;
}
int main(int argc, char *argv[])
{
    int pid;
    int pattern = atoi(argv[1]);
    rwlock_init();
    int repeat_numebr = determine_siz(&pattern) - 1;
    pid = fork();
    int mode = shifter(&pattern);
    for (int i = 0; i < repeat_numebr - 1; i++)
    {
        if (pid == 0)
            break;
        if (pid > 0)
        {
            mode = shifter(&pattern);
            pid = fork();
        }
    }

    if (pid == 0)
        rwlock_test(mode);

    else if (pid > 0)
    {
        for (int i = 0; i < NUMBER_OF_CHILD + 1; ++i)
            wait();
        printf(1, "user program finished\n");
    }
    exit();
}
