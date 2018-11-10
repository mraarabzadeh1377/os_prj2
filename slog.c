#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[])
{
    int pid = atoi(argv[1]);
    invoked_systemcall(pid);
    printf(1, "hello beach\n");
    exit();
}
