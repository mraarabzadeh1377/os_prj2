#define EMPTY 0
#define WRITING 1
#define READING 2

struct readQueue
{
    int pid;
    struct readQueue *next;
};

struct writeQueue
{
    int pid;
    struct writeQueue *next;
};

struct rwlock
{
    struct spinlock lk; // spinlock protecting this rw lock
    struct readQueue *readQueue;
    struct writeQueue *writeQueue;
    uint mode; // 0: empty     1: write      2: read
    uint writeQueueSize;
    uint readQueueSize;

    // For debugging:
    char *name; // Name of lock.
};
