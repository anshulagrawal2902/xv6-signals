#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"


int main(int argc , char* argv[]){
    kill1(atoi(argv[0]), atoi(argv[1]));
    exit();
}