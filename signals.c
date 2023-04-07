#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "signals.h"

extern void dh_sigkill(int);
extern void dh_sigstop(int);
extern void dh_sigint(int);
extern void dh_sigcont(int);

static void(*defaultHandlers[])(int) = {
    [SIGKILL] dh_sigkill,
    [SIGSTOP] dh_sigstop,
    [SIGINT]  dh_sigint,
    [SIGCONT] dh_sigcont,
};

// function that runs default signal handler in kernel stack
int doDefaultSignal(int signo){
  defaultHandlers[signo](signo);
  return 0;
}

//funtion defined to run user handlers in user stack
int doSignal(int signo){
  return 0;
}