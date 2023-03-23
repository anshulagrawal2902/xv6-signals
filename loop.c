#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main(int argc , char* argv[]){
    int a = getpid();
    for(;;)
    {
        printf(1, "%d", a);
        sleep(1);
    }   
 exit();
}