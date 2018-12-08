#include "types.h"
#include "user.h"
#define NUMBER_OF_CHILD 10

int main(int argc, char *argv[])
{
    int pid;
    ticketlock_init();
    pid = fork();
    for (int i = 0; i < NUMBER_OF_CHILD; i++)
        if (pid > 0)
            pid = fork();

    if (pid == 0)
        ticketlock_test();

    else if (pid > 0)
    {
        for (int i = 0; i < NUMBER_OF_CHILD + 1; ++i)
            wait();
        printf(1, "user program finished\n");
    }
    exit();
}
