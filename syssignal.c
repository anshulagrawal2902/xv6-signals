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

