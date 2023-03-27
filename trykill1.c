#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "signals.h"

void print(int signo){
    printf(1, "inside user");
}
void temp(int signo){
    
}
void temp2(int signo){
    
}

int main(int argc , char* argv[]){
    int a = getpid();
    printf(1, "printing pid in user land %d\n", a);
    signal(SIGKILL, &temp); 
    printf(1, "funciton call adddress %x \n", &temp);
    printf(1, "funciton call adddress %x \n", &temp2);
    printf(1, "funciton call adddress %x \n", &print);

    for(int  i =0; i < 10; i++){
        printf(1, "%d \n", i);
    }
    kill1(a, SIGKILL);
    
    for(int i  = 20; i < 30; i++ ){
        printf(1, "%d \n", i);
    }
    exit();
}