// rw locks

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "rwlock.h"

void initRwlock(struct rwlock *lk, char *name)
{
    initlock(&lk->lk, "rw lock");
    lk->name = name;
    lk->mode = EMPTY;
    lk->writeQueueSize = 0;
    lk->readQueueSize = 0;
}

void addToReadQueue(struct rwlock *lk)
{
    // first Initial
    struct readQueue *queueIterator = lk->readQueue;
    struct readQueue *newProc = (struct readQueue *)kalloc();
    newProc->pid = myproc()->pid;
    newProc->next = 0;

    if (!lk->readQueueSize)
        lk->readQueue = newProc;
    else
    {
        for (int i = 1; i < lk->readQueueSize; i++)
            queueIterator = queueIterator->next;

        queueIterator->next = newProc;
    }
}

void addToWriteQueue(struct rwlock *lk)
{
    // first Initial
    struct writeQueue *queueIterator = lk->writeQueue;
    struct writeQueue *newProc = (struct writeQueue *)kalloc();
    newProc->pid = myproc()->pid;
    newProc->next = 0;

    if (!lk->writeQueueSize)
        lk->writeQueue = newProc;
    else
    {
        for (int i = 1; i < lk->writeQueueSize; i++)
            queueIterator = queueIterator->next;

        queueIterator->next = newProc;
    }
}

void rwWait(struct rwlock *lk, int isWriter)
{
    acquire(&lk->lk);

    if (isWriter)
    {
        addToWriteQueue(lk);
        lk->writeQueueSize++;
        cprintf("   writequeue : %d  \n", lk->writeQueueSize);

        if (lk->mode == EMPTY)
            lk->mode = WRITING;
        else
            sleep(lk, &lk->lk);
    }
    else
    {
        addToReadQueue(lk);
        lk->readQueueSize++;

        if (lk->mode == EMPTY)
            lk->mode = READING;
        else if (lk->mode == WRITING)
            sleep(lk, &lk->lk);
    }

    cprintf("   lock mode  : %s  \n", !lk->mode ? "empty" : (lk->mode == 1) ? "writing" : "reading");
    cprintf("   readqueue : %d  \n", lk->readQueueSize);
    cprintf("   writequeue : %d  \n", lk->writeQueueSize);
    cprintf("   mode : %s  \n", isWriter ? "writing" : "reading");
    cprintf("   entered lock  => pid : %d \n\n", myproc()->pid);

    release(&lk->lk);
}

void getProcInfoFromQueue(struct rwlock *lk, uint *isInQueue, uint *isWriter)
{

    *(isInQueue) = 0;

    // check in read queue
    struct readQueue *rQueueIterator = lk->readQueue;

    for (int i = 0; i < lk->readQueueSize; i++)
    {
        if (rQueueIterator->pid == myproc()->pid)
        {
            *(isInQueue) = 1;
            *(isWriter) = 0;
            return;
        }
        rQueueIterator = rQueueIterator->next;
    }

    // check in read queue
    struct writeQueue *wQueueIterator = lk->writeQueue;

    for (int i = 0; i < lk->writeQueueSize; i++)
    {
        if (wQueueIterator->pid == myproc()->pid)
        {
            *(isInQueue) = 1;
            *(isWriter) = 1;
            return;
        }
        wQueueIterator = wQueueIterator->next;
    }
}

void rwSignal(struct rwlock *lk)
{
    acquire(&lk->lk);
    uint isInQueue, isWriter;
    getProcInfoFromQueue(lk, &isInQueue, &isWriter);
    if (isInQueue)
    {
        if (isWriter)
        {
            // remove from queue
            lk->writeQueue = lk->writeQueue->next;
            lk->writeQueueSize--;

            if (lk->readQueueSize)
            {
                // change mode
                lk->mode = READING;
                // waking up all the reading requests
                struct readQueue *readQueueIterator = lk->readQueue;
                for (int i = 0; i < lk->readQueueSize; i++)
                {
                    wakeupByPid(lk, readQueueIterator->pid);
                    readQueueIterator = readQueueIterator->next;
                }
            }
            else if (lk->writeQueueSize)
            {
                // waking up the first writing request
                wakeupByPid(lk, lk->writeQueue->pid);
            }
            else
            {
                // change mode
                lk->mode = EMPTY;
            }
        }
        else
        {
            // remove form queue
            struct readQueue *readQueueIterator = lk->readQueue;
            if (lk->readQueue->pid == myproc()->pid)
            {
                lk->readQueue = lk->readQueue->next;
            }
            else
            {
                for (int i = 1; i < lk->readQueueSize; i++)
                {
                    if (readQueueIterator->next->pid == myproc()->pid)
                    {
                        readQueueIterator->next = readQueueIterator->next->next;
                        break;
                    }
                    readQueueIterator = readQueueIterator->next;
                }
            }
            lk->readQueueSize--;

            if (!lk->readQueueSize && lk->writeQueueSize)
            {
                lk->mode = WRITING;
                // waking up the first writing request
                wakeupByPid(lk, lk->writeQueue->pid);
            }
            else if (!lk->readQueueSize && !lk->writeQueueSize)
            {
                lk->mode = EMPTY;
            }
        }
    }
    cprintf("\nrelease lock  => pid : %d \n", myproc()->pid);
    cprintf("mode : %s  \n\n", isWriter ? "writing" : "reading");

    release(&lk->lk);
}
