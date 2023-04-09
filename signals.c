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
extern void dh_sigterm(int);
extern void dh_sigusr1(int);
extern void dh_sigsegv(int);
extern void dh_sigchld(int);
extern void dh_sigill(int);
extern void dh_sigvtalrm(int);

static void(*defaultHandlers[])(int) = {
    [SIGKILL]   dh_sigkill,
    [SIGSTOP]   dh_sigstop,
    [SIGINT]    dh_sigint,
    [SIGCONT]   dh_sigcont,
    [SIGTERM]   dh_sigterm,
    [SIGUSR1]   dh_sigusr1,
    [SIGSEGV]   dh_sigsegv,
    [SIGCHLD]   dh_sigchld,
    [SIGILL]    dh_sigill,
    [SIGVTALRM] dh_sigvtalrm,

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