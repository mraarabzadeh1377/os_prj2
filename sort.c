#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[])
{
    int numArr[10];
    int i, j, temp, fd;

    for (i = 0; i < 5; i++)
        numArr[i] = atoi(argv[i + 1]);

    for (i = 0; i < 5; i++)
        for (j = i; j < 5; j++)
            if (numArr[i] > numArr[j])
            {
                temp = numArr[j];
                numArr[j] = numArr[i];
                numArr[i] = temp;
            }

    fd = open("result.txt", O_CREATE | O_WRONLY);
    if (!fork())
    {
        printf(1, "%d ", inc_num(2));
        while (1)
        {
        }
    }

    for (j = 0; j < 5; j++)
        printf(fd, "%d ", numArr[j]);
    printf(fd, "\n");

    close(fd);
    exit();
}
