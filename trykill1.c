#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "signals.h"

void print(int signo){
    printf(1, "inside user\n");
}
void temp(int signo){
    // printf(1, "inside user defined signal handler\n");
    for(int i = 1000; i < 1010 ; i++){
        printf(1, "inides user handler %d \n", i);
    }
    // print(signo);
    // exit();
}
void temp2(int signo){
    
}

int main(int argc , char* argv[]){
    // int a = getpid();

    struct sigset_t mask;
    struct sigset_t oldmask;

    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    // printf(1, "printing pid in user land %d\n", a);

    // signal(SIGKILL, &temp); 

    // printf(1, "funciton call adddress %x \n", &temp);
    // printf(1, "funciton call adddress %x \n", &temp2);
    // printf(1, "funciton call adddress %x \n", &print);

    // for(int  i =0; i < 100; i++){
    //     printf(1, "%d \n", i);
    // }

    // kill1(a, SIGKILL);
    
    // for(int i  = 200; i < 300; i++ ){
    //     printf(1, "%d \n", i);
    // }

    // int b = fork();
    // if(b == 0){
    //     // signal(SIGSTOP, &temp);
    //     // int a = getpid();
    //     // while(1){
    //     for(int i = 400; i < 500; i++){
    //         printf(1, "%d \n", i);
    //     }
    //         sleep(1);
    //         printf(1, "i m child running\n");
    //         kill1(a, SIGKILL);

    //     for(int i = 100; i < 110 ; i++){
    //         printf(1, "%d \n", i);
    //     }
    //     // }
    // }
    // else{
    //     // pause();
    //     for(int i = 0; i < 1000; i++){
    //         printf(1, "%d \n", i);
    //     }
    //     // kill1(b,SIGKILL);
        
    // }
    exit();
}