#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "signals.h"

// void print(int signo){
//     printf(1, "inside user\n");
// }

void handler(int signo) {
    printf(1, "Signal SIGSTOP received in user space\n");
}

void usr_sig_handler(int signo){
    // printf(1, "Inides user sihnal handler\n");
    printf(1, "Signal SIGSTOP received in user space\n");
}
void temp2(int signo){
    
}

int main(int argc , char* argv[]){
    int a = getpid();
    printf(1, "Starting the program\n");
    printf(1, "Printing pid in user land %d\n", a);

    int pid;
    signal(SIGSTOP, &usr_sig_handler); 
    pid = fork();
    if (pid == 0) {
        // child process
        printf(1, "Child process started with pid %d\n", getpid());
        sleep(1);
        kill1(a, SIGSTOP);
        pause();
        printf(1, "Child process ended\n");
        exit();
    } else if (pid > 0) {
        pause();
        // parent process
        printf(1, "Parent process started with pid %d\n", getpid());
        printf(1, "Parent process resumed after receiving signal\n");
        kill1(pid, SIGKILL);
        printf(1, "Parent process ended\n");
        exit();
    } else {
        // fork failed
        printf(1, "Fork failed\n");
        exit();
    }
    
    // printf(1, "funciton call adddress %x \n", &temp);
    // printf(1, "funciton call adddress %x \n", &temp2);
    // printf(1, "funciton call adddress %x \n", &print);

    // // Test case 1: Send a signal to the current process
    // printf(1, "Test case 1: Send SIGKILL to self\n");
    // signal(SIGSTOP, &handler);
    // kill1(a, SIGKILL);

    // // Test case 2: Send a signal to a non-existent process
    // printf(1, "Test case 2: Send SIGINT to non-existent process\n");
    // if (kill1(9999, SIGINT) < 0) {
    //     printf(1, "kill1 system call failed as expected\n");
    // }

    // // Test case 3: Send a signal to an invalid process ID
    // printf(1, "Test case 3: Send SIGINT to invalid process ID -1\n");
    // if (kill1(-1, SIGINT) < 0) {
    //     printf(1, "kill1 system call failed as expected\n");
    // }

    // // Test case 4: Send a signal and pause indefinitely
    // // printf(1, "Test case 4: Send SIGINT and pause indefinitely\n");
    // // signal(SIGINT, handler);
    // // printf(1, "Pausing indefinitely...\n");
    // // pause();

    // // Test case 5: Send a signal and pause for 5 seconds
    // printf(1, "Test case 5: Send SIGUSR1 and pause for 5 seconds\n");
    // signal(SIGKILL, &handler);
    // printf(1, "Pausing for 5 seconds...\n");
    // // alarm(5);
    // pause();



    // for(int  i =0; i < 10; i++){
    //     printf(1, "%d \n", i);
    // }
    // kill1(a, SIGKILL);  
    
    // for(int i  = 200; i < 300; i++ ){
    //     printf(1, "%d \n", i);
    // }

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
