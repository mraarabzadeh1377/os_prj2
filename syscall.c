#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if (addr >= curproc->sz || addr + 4 > curproc->sz)
    return -1;
  *ip = *(int *)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if (addr >= curproc->sz)
    return -1;
  *pp = (char *)addr;
  ep = (char *)curproc->sz;
  for (s = *pp; s < ep; s++)
  {
    if (*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4 * n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();

  if (argint(n, &i) < 0)
    return -1;
  if (size < 0 || (uint)i >= curproc->sz || (uint)i + size > curproc->sz)
    return -1;
  *pp = (char *)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int argstr(int n, char **pp)
{
  int addr;
  if (argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_inc_num(void);
extern int sys_inc_num(void);
extern int sys_invoked_systemcall(void);
extern int sys_sort_systemcall(void);
extern int sys_get_count(void);
extern int sys_log_systemcall(void);

static int (*syscalls[])(void) = {
    [SYS_fork] sys_fork,
    [SYS_exit] sys_exit,
    [SYS_wait] sys_wait,
    [SYS_pipe] sys_pipe,
    [SYS_read] sys_read,
    [SYS_kill] sys_kill,
    [SYS_exec] sys_exec,
    [SYS_fstat] sys_fstat,
    [SYS_chdir] sys_chdir,
    [SYS_dup] sys_dup,
    [SYS_getpid] sys_getpid,
    [SYS_sbrk] sys_sbrk,
    [SYS_sleep] sys_sleep,
    [SYS_uptime] sys_uptime,
    [SYS_open] sys_open,
    [SYS_write] sys_write,
    [SYS_mknod] sys_mknod,
    [SYS_unlink] sys_unlink,
    [SYS_link] sys_link,
    [SYS_mkdir] sys_mkdir,
    [SYS_close] sys_close,
    [SYS_inc_num] sys_inc_num,
    [SYS_invoked_systemcall] sys_invoked_systemcall,

    [SYS_sort_systemcall] sys_sort_systemcall,
    [SYS_get_count] sys_get_count,
    [SYS_log_systemcall] sys_log_systemcall,

};
//create function for saving systemcall datas
char *give_systemcall_name(int num)
{
  switch (num)
  {
  case 1:
    return "fork\0";
  case 2:
    return "exit\0";
  case 3:
    return "wait\0";
  case 4:
    return "pipe\0";
  case 5:
    return "read\0";
  case 6:
    return "kill\0";
  case 7:
    return "exec\0";
  case 8:
    return "fstat\0";
  case 9:
    return "chdir\0";
  case 10:
    return "dup\0";
  case 11:
    return "getpid\0";
  case 12:
    return "sbrk\0";
  case 13:
    return "sleep\0";
  case 14:
    return "uptime\0";
  case 15:
    return "open\0";
  case 16:
    return "write\0";
  case 17:
    return "mknode\0";
  case 18:
    return "unlink\0";
  case 19:
    return "link\0";
  case 20:
    return "mkdir\0";
  case 21:
    return "close\0";
  case 22:
    return "inc_num\0";
  case 23:
    return "invoked_systemcall\0";
  case 24:
    return "sort_systemcall\0";
  case 25:
    return "get_count\0";
  case 26:
    return "log_systemcall\0";
    break;
  }
  return "";
}

void save_systemcall_data(struct proc *curproc, int systemcall_number)
{
  struct systemcall_base_inf *sc;
  sc = (struct systemcall_base_inf *)kalloc();
  if (!curproc->systemcalls[systemcall_number])
  {
    sc->id = systemcall_number;
    sc->name = give_systemcall_name(systemcall_number);
    sc->number_of_call = 0;
    sc->instances = 0;
    curproc->systemcalls[systemcall_number] = sc;
  }

  // struct real_time_systemcall_data* rtnext= curproc->all_systemcall_history_data[systemcall_number]->this_systemcall_data;
  // for (int i = 0; i < curproc->all_systemcall_history_data[systemcall_number]->number_of_call; ++i)
  // {
  //   rtnext=rtnext->next;
  // }
  // struct real_time_systemcall_data nrt;
  // struct rtcdate *rtcdate_temp=0;
  // cmostime(rtcdate_temp);
  // nrt.systemcall_time=rtcdate_temp;
  // cmostime(nrt.systemcall_time);
  // nrt.next=0;
  // rtnext=&nrt;

  curproc->systemcalls[systemcall_number]->number_of_call += 1;
}
void syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;

  if (num > 0 && num < NELEM(syscalls) && syscalls[num])
  {

    curproc->tf->eax = syscalls[num]();
    save_systemcall_data(curproc, num);
    // cprintf("sysnum is : %d %d %s\n", curproc->pid,curproc->all_systemcall_history_data[num]->number_of_call,give_systemcall_name(num));
  }
  else
  {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
