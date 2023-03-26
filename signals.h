#define SIGKILL 1
#define SIGSTOP 2
#define SIGINT 3

typedef void(*signalHandler)(int);

struct sigaction
{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, int *, void *);
    int sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};  



