#include"param.h"
#define SIG_DFL ((signalHandler) 0)
#define SIGVTALRM 0
#define SIGKILL   1
#define SIGSTOP   2
#define SIGINT    3
#define SIGCONT   4
#define SIGTERM   5
#define SIGUSR1   6
#define SIGSEGV   7
#define SIGCHLD   8
#define SIGILL    9


#define SIG_BLOCK     100
#define SIG_UNBLOCK   101
#define SIG_SETMASK   102

#define ITIMER_PROF     200
#define ITIMER_REAL     201
#define ITIMER_VIRTUAL  202

int doDefaultSignal(int signo);
int doSignal(int signo);

typedef void(*signalHandler)(int);

struct sigset_t{
    unsigned long int mask[MAX_SIGNALS];
};

struct timeval{
    int  tv_sec;  //in seconds
    int tv_usec;  //in microseconds
};

struct itimerval{
    struct timeval it_interval; //timer interval
    struct timeval it_value;    //expiration time
};

struct sigaction
{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, int *, void *);
    int sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};



