#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "signals.h"

int sys_kill1(void){

  int pid, signum;
  if(argint(0, &pid) < 0 || argint(1, &signum) < 0)
    return -1;
  return kill1(pid, signum);
}

// int sys_sigaction(void){

// }

int sys_signal(void){
  int signo;
  void (*fn)(int);
  if(argint(0, &signo) < 0 || argfun(1, &fn, sizeof(fn)) < 0){
    return signal(signo, fn);
  }
  return 0;
}

