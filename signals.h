#include"param.h"
#define SIGKILL 1
#define SIGSTOP 2
#define SIGINT 3
#define SIGCONT 4

#define SIG_DFL ((signalHandler) 0)

#define SIG_BLOCK 100
#define SIG_UNBLOCK 101
#define SIG_SETMASK 102

int doDefaultSignal(int signo);
int doSignal(int signo);


typedef void(*signalHandler)(int);


struct sigset_t{
    unsigned long int mask[MAX_SIGNALS];
};

struct sigaction
{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, int *, void *);
    int sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};



