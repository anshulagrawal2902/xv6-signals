#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "signals.h"

extern int pause_chan;

int sys_kill1(void){

  int pid, signum;
  if(argint(0, &pid) < 0 || argint(1, &signum) < 0)
    return -1;
  return kill1(pid, signum);
}

int sys_pause(void){
  return pause(&pause_chan);
}

int sys_signal(void){
  int signo;
  void (*fn)(int);
  if(argint(0, &signo) < 0 || argfun(1, &fn, sizeof(fn)) < 0){
    return -1;
  }
  return signal(signo, fn);
}

int sys_sigprocmask(void){
  int how;
  struct sigset_t* set;
  struct sigset_t* oldset;
  if(argint(0, &how) < 0 || argptr(1, (void*)&set, sizeof(set)) < 0  || argptr(2, (void*)&oldset, sizeof(oldset)) < 0 ){
    return -1;
  }
  return sigprocmask(how, set, oldset);
  // cprintf("inside sys_sigprocmask \n");
  // return 0;
}

