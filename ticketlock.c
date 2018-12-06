// Sleeping locks

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "ticketlock.h"

void initTicketlock(struct ticketlock *lk, char *name)
{
    initlock(&lk->lk, "ticket lock");
    lk->name = name;
    lk->numberOfRequests = 0;
    lk->requestsList = (struct request *)kalloc();
}

void addProcToTicketlist(struct ticketlock *lk)
{
    struct request *requestIterator = lk->requestsList;
    struct request *temp = (struct request *)kalloc();
    temp->pid = myproc()->pid;
    temp->next = 0;

    if (lk->numberOfRequests == 0)
        lk->requestsList = temp;
    else
    {
        for (int i = 1; i < lk->numberOfRequests; i++)
            requestIterator = requestIterator->next;

        requestIterator->next = temp;
    }
    lk->numberOfRequests++;
}

void removeProcFromTicketlist(struct ticketlock *lk)
{
    lk->requestsList = lk->requestsList->next;
    lk->numberOfRequests -= 1;
}

void acquireTicket(struct ticketlock *lk)
{
    acquire(&lk->lk);

    addProcToTicketlist(lk);

    while (lk->requestsList->pid != myproc()->pid)
        sleep(lk, &lk->lk);

    release(&lk->lk);
}

void releaseTicket(struct ticketlock *lk)
{
    acquire(&lk->lk);
    if (lk->numberOfRequests && lk->requestsList->pid == myproc()->pid)
    {
        removeProcFromTicketlist(lk);
        wakeup(lk);
    }
    release(&lk->lk);
}
