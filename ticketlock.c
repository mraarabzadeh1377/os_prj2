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
    lk->name = name;
    lk->turn = 0;
    lk->lastTicketNumber = 0;
}

void acquireTicket(struct ticketlock *lk)
{
    int myTurn = fetch_and_add(&lk->lastTicketNumber, 1);

    while (lk->turn != myTurn)
        ticketlockSleep(lk);
}

void releaseTicket(struct ticketlock *lk)
{
    fetch_and_add(&lk->turn, 1);
    wakeup(lk);
}
