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
    for(int i = 0; i < 10 ; i++){
        printf(1, "inides user handler %d \n", i);
    }
    // print(signo);
    // exit();
}
void temp2(int signo){
    
}

int main(int argc , char* argv[]){
    int a = getpid();
    printf(1, "printing pid in user land %d\n", a);

    signal(SIGSTOP, &temp); 

    // printf(1, "funciton call adddress %x \n", &temp);
    // printf(1, "funciton call adddress %x \n", &temp2);
    // printf(1, "funciton call adddress %x \n", &print);

    for(int  i =0; i < 10; i++){
        printf(1, "%d \n", i);
    }
    kill1(a, SIGSTOP);
    
    for(int i  = 20; i < 30; i++ ){
        printf(1, "%d \n", i);
    }

    // int b = fork();
    // if(b == 0){
    //     signal(SIGSTOP, &temp);
    //     int a = getpid();
    //     while(1){
    //         sleep(1);
    //         printf(1, "i m child running\n");
    //         kill1(a, SIGSTOP);
    //     }
    // }
    // else{
    //     for(int i = 0; i < 10 ; i++){
    //         printf(1, "%d \n", i);
    //     }
    //     kill1(b,SIGSTOP);
    //     for(int i = 20; i < 30 ; i++){
    //         printf(1, "%d \n", i);
    //     }
    // }
    exit();
}