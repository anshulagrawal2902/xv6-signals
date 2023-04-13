#include "types.h"
#include "stat.h"
#include "user.h"
#include "signals.h"

void sig_usr_hdlr(int a){
    for(int i = 0; i < 10; i++){
        printf(1, "this is user handler running 1 %d \n", i);
    }
}

void sig_usr_hdlr2(int a){
    for(int i = 0; i < 10; i++){
        printf(1, "this is user handler running 2 %d \n", i);
    }
}

int
main(void)
{
    int a = getpid();
    signal(SIGUSR1, sig_usr_hdlr);
    signal(SIGTERM, sig_usr_hdlr2);
    for(int i = 0; i < 20; i++){
        printf(1, "before signal %d \n", i);
    }
    kill1(a, SIGUSR1);
    for(int i = 0; i < 20; i++){
        printf(1, "after signal1 %d \n", i);
    }
    kill1(a, SIGTERM);
    for(int i = 0; i < 20; i++){
        printf(1, "after signal2 %d \n", i);
    }
    exit();
}
