#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"
#include "date.h"
// #include "user.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.
#define INT "int"
#define FD "fd"
#define CHARS "char*"
#define INTS "int*"
#define SHORT "short"
#define STRUCTS "struct stat*"
#define VOIDS "void*"
#define CHARSS "char**"

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
extern int sys_ticketlock_init(void);
extern int sys_ticketlock_test(void);
extern int sys_rwlock_init(void);
extern int sys_rwlock_test(void);

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

    [SYS_ticketlock_init] sys_ticketlock_init,
    [SYS_ticketlock_test] sys_ticketlock_test,
    [SYS_rwlock_init] sys_rwlock_init,
    [SYS_rwlock_test] sys_rwlock_test,
};
//create function for saving systemcall datas
struct systemcall_info
{
  char *arg_type[2];
  int number_of_parameter;
  char *name;
};

void give_systemcall_info(int num, struct systemcall_base_inf *systemcall)
{
  switch (num)
  {
  case 1:
    systemcall->name = "fork\0";
    systemcall->parameter_number = 0;
    return;
  case 2:
    systemcall->name = "exit\0";
    systemcall->parameter_number = 0;
    return;
  case 3:
    systemcall->name = "wait\0";
    systemcall->parameter_number = 0;
    return;
  case 4:
    systemcall->name = "pipe\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = INTS;
    return;
  case 5:
    systemcall->name = "read\0";
    systemcall->parameter_number = 3;
    systemcall->arg_type[0] = FD;
    systemcall->arg_type[1] = VOIDS;
    systemcall->arg_type[2] = INT;
    return;
  case 6:
    systemcall->name = "kill\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = INT;
    return;
  case 7:
    systemcall->name = "exec\0";
    systemcall->parameter_number = 2;
    systemcall->arg_type[0] = CHARS;
    systemcall->arg_type[1] = CHARSS;
    return;
  case 8:
    systemcall->name = "fstat\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = FD;
    systemcall->arg_type[1] = STRUCTS;

    return;
  case 9:
    systemcall->name = "chdir\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = CHARS;
    return;
  case 10:
    systemcall->name = "dup\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = FD;
    return;
  case 11:
    systemcall->name = "getpid\0";
    systemcall->parameter_number = 0;
    return;
  case 12:
    systemcall->name = "sbrk\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = INT;
    return;
  case 13:
    systemcall->name = "sleep\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = INT;
    return;
  case 14:
    systemcall->name = "uptime\0";
    systemcall->parameter_number = 0;
    return;
  case 15:
    systemcall->name = "open\0";
    systemcall->parameter_number = 2;
    systemcall->arg_type[0] = CHARS;
    systemcall->arg_type[1] = INT;
    return;
  case 16:
    systemcall->name = "write\0";
    systemcall->parameter_number = 3;
    systemcall->arg_type[0] = FD;
    systemcall->arg_type[1] = VOIDS;
    systemcall->arg_type[2] = INT;
    return;
  case 17:
    systemcall->name = "mknode\0";
    systemcall->parameter_number = 3;
    systemcall->arg_type[0] = CHARS;
    systemcall->arg_type[1] = SHORT;
    systemcall->arg_type[2] = SHORT;
    return;
  case 18:
    systemcall->name = "unlink\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = CHARS;
    return;
  case 19:
    systemcall->name = "link\0";
    systemcall->parameter_number = 2;
    systemcall->arg_type[0] = CHARS;
    systemcall->arg_type[1] = CHARS;
    return;
  case 20:
    systemcall->name = "mkdir\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = CHARS;
    return;
  case 21:
    systemcall->name = "close\0";
    systemcall->arg_type[0] = INT;
    systemcall->parameter_number = 1;
    return;
  case 22:
    systemcall->name = "inc_num\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = INT;
    return;
  case 23:
    systemcall->name = "invoked_systemcall\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = INT;
    return;
  case 24:
    systemcall->name = "sort_systemcall\0";
    systemcall->parameter_number = 1;
    systemcall->arg_type[0] = INT;
    return;
  case 25:
    systemcall->name = "get_count\0";
    systemcall->parameter_number = 2;
    systemcall->arg_type[0] = INT;
    systemcall->arg_type[1] = INT;
    return;
  case 26:
    systemcall->name = "log_systemcall\0";
    systemcall->parameter_number = 0;
    return;
  case 27:
    systemcall->name = "ticketlock_init\0";
    systemcall->parameter_number = 0;
    return;
  case 28:
    systemcall->name = "ticketlock_test\0";
    systemcall->parameter_number = 0;
    return;
  }
  return;
}

void save_systemcall_data(struct proc *curproc, int systemcall_number)
{
  struct systemcall_base_inf *sbi = (struct systemcall_base_inf *)kalloc();
  // initial instance on systemcall
  struct systemcall_instance *new_si = (struct systemcall_instance *)kalloc();
  struct rtcdate *temp_time = (struct rtcdate *)kalloc();
  cmostime(temp_time);
  new_si->time = temp_time;
  give_systemcall_info(systemcall_number, sbi);

  // save systemcalls arguments
  for (int i = 0; i < sbi->parameter_number; i++)
  {
    new_si->arg_value[i] = (struct argumnet_value *)kalloc();
    new_si->arg_value[i]->chars_val = (char *)kalloc();
    new_si->arg_value[i]->pointer_val = (char **)kalloc();

    if (!strncmp(sbi->arg_type[i], STRUCTS, 12))
      argptr(i, new_si->arg_value[i]->pointer_val, sizeof(struct stat *));

    else if (!strncmp(sbi->arg_type[i], INTS, 4))
      argptr(i, new_si->arg_value[i]->pointer_val, sizeof(int *));

    else if (!strncmp(sbi->arg_type[i], CHARSS, 6))
      argptr(i, new_si->arg_value[i]->pointer_val, sizeof(char **));

    else if (!strncmp(sbi->arg_type[i], CHARS, 5))
    {
      argstr(i, &(new_si->arg_value[i]->chars_val));
    }

    else if (!strncmp(sbi->arg_type[i], INT, 3) || !strncmp(sbi->arg_type[i], SHORT, 5) || !strncmp(sbi->arg_type[i], FD, 2))
      argint(i, &(new_si->arg_value[i]->int_val));
  }

  if (!curproc->systemcalls[systemcall_number])
  {
    //initial systemcall in pcb for firsttime
    sbi->id = systemcall_number;
    sbi->number_of_call = 0;
    sbi->instances = new_si;
    curproc->systemcalls[systemcall_number] = sbi;
  }
  else
  {
    struct systemcall_instance *si_iterator = curproc->systemcalls[systemcall_number]->instances;
    for (int i = 0; i < curproc->systemcalls[systemcall_number]->number_of_call - 1; i++)
      si_iterator = si_iterator->next;

    si_iterator->next = new_si;
  }

  curproc->systemcalls[systemcall_number]
      ->number_of_call += 1;
}

void syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;

  if (num > 0 && num < NELEM(syscalls) && syscalls[num])
  {

    curproc->tf->eax = syscalls[num]();
    // save_systemcall_data(curproc, num);
  }
  else
  {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
