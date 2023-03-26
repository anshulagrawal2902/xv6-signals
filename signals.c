#include "signals.h"
#include "proc.h"

extern void dh_sigkill(int);
extern void dh_sigstop(int);
extern void dh_sigint(int);

static void(*defaultHandlers[])(int) = {
    [SIGKILL] dh_sigkill,
    [SIGSTOP] dh_sigstop,
    [SIGINT]  dh_sigint,
};

int doSignal(int signo){
  struct proc* curproc  = myproc();
  if(curproc->hasUserHandler[signo] == 1){
    // TODO :switch stack to process stack and run the function pointed by signal handler
  }
  else{
    defaultHandlers[signo](signo);
  }
  if(curproc->pid == 1 && signo==SIGKILL || signo==SIGSTOP){}
  else if(signo == SIGKILL)
    kill(curproc->pid);
}