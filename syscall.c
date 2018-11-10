#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"
#include "lapic.c"
//#include <string.h>

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
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

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_inc_num] sys_inc_num,
};
//create function for saving systemcall datas
char * give_systemcall_name(int num)
{
  switch(num)
  {
    case 1:return "fork";
    case 2:return "exit";
    case 3:return "wait";
    case 4:return "pipe";
    case 5:return "read";
    case 6:return "kill";
    case 7:return "exec";
    case 8:return "fstat";
    case 9:return "chdir";
    case 10:return "dup";
    case 11: return "getpid";
    case 12: return "sbrk";
    case 13: return "sleep";
    case 14: return "uptime";
    case 15: return "open";
    case 16: return "write";
    case 17: return "mknode";
    case 18: return "unlink";
    case 19: return "link";
    case 20: return "mkdir";
    case 21: return "close";
    case 22: return "inc_num";
    case 23: return "";
    case 24: return"";
    case 25: return"";
    case 26: return"";
    break;
  }
  return "";
}

void save_systemcall(struct proc *curproc,int systemcall_number)
{
  if (!curproc->all_systemcall_history_data[systemcall_number])
  {
    struct systemcall_base_inf sc;
    sc.tmux=1;
    sc.systemcall_id=systemcall_number;
    //strcpy(sc.systemcall_name,give_systemcall_name(systemcall_number));
    sc.number_of_call=0;
    sc.this_systemcall_data=0;
    curproc->all_systemcall_history_data[systemcall_number]=&sc;
  }
  struct real_time_systemcall_data* rtnext= 
              curproc->all_systemcall_history_data[systemcall_number]->this_systemcall_data;
  for (int i = 0; i < curproc->all_systemcall_history_data[systemcall_number]->number_of_call; ++i)
  {
    rtnext=rtnext->next;
  }
  struct real_time_systemcall_data nrt;
  cmostime(nrt.systemcall_time);
  nrt.next=NULL;
  rtnext=&nrt;
  curproc->all_systemcall_history_data[systemcall_number]->number_of_call++;      
}
void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;

  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
