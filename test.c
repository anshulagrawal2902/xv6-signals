#include "types.h"
#include "stat.h"
#include "user.h"
#include "signals.h"

int b, c;
void sig_usr_hdlr(int a){
    int ans = b + c;
    for(int i = 0; i < 10; i++){
        printf(1, "this is user handler running 1 %d \n", ans);
    }
}

void sig_usr_hdlr2(int a){
    for(int i = 0; i < 10; i++){
        printf(1, "this is user handler running 2 %d \n", i);
    }
}
    int var = 10;

int
main(void)
{
    b = 2;
    c = 3;
    int pid;
    pid = fork();

    if(pid == 0){
        pause();
        printf(1, "value of var is %d \n", var );
        printf(1, "this runs after parent\n");

        // signal(SIGUSR1, sig_usr_hdlr2);
        // signal(SIGTERM, sig_usr_hdlr);
        // for(int i = 0; i < 50; i++){
        //     printf(1, "child process %d\n", i );
        // }
        // exit();
    }
    else{
        sleep(2);
        var = 20;
        printf(1, "This should run first\n");
        kill1(pid, SIGUSR1);
        // kill1(pid, SIGUSR1);
        // kill1(pid, SIGTERM);
        exit();
    }


    // for(int i = 0; i < 20; i++){
    //     printf(1, "before signal %d \n", i);
    // }

    // kill1(a, SIGTERM);
    // kill1(a, SIGUSR1);
    

    // for(int i = 0; i < 100; i++){
    //     printf(1, "after signals %d \n", i);
    // }

    exit();
}
