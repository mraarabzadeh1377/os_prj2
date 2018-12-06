
struct request
{
    int pid;
    struct request *next;
};

struct ticketlock
{
    struct spinlock lk; // spinlock protecting this ticket lock
    struct request *requestsList;
    int numberOfRequests;

    // For debugging:
    char *name; // Name of lock.
};
