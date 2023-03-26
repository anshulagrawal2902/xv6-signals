#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "signals.h"


int main(int argc , char* argv[]){
    int a = getpid();
    for(int  i =0; i < 10; i++){
        printf(1, "%d \n", i);
    }

    kill1(a, SIGKILL);

    for(int i  = 20; i < 30; i++ ){
        printf(1, "%d \n", i);
    }
    exit();
}