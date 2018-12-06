// #include "types.h"
// #include "stat.h"
// #include "user.h"
// #include "fcntl.h"

// int main(int argc, char *argv[])
// {
//     int numArr[10];
//     int i, j, temp, fd;

//     for (i = 0; i < 5; i++)
//         numArr[i] = atoi(argv[i + 1]);

//     for (i = 0; i < 5; i++)
//         for (j = i; j < 5; j++)
//             if (numArr[i] > numArr[j])
//             {
//                 temp = numArr[j];
//                 numArr[j] = numArr[i];
//                 numArr[i] = temp;
//             }

//     fd = open("result.txt", O_CREATE | O_WRONLY);
//     if (!fork())
//     {
//         printf(1, "%d ", inc_num(2));
//         while (1)
//         {
//         }
//     }

//     for (j = 0; j < 5; j++)
//         printf(fd, "%d ", numArr[j]);
//     printf(fd, "\n");

//     close(fd);
//     exit();
// }

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