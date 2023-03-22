#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main(int argc , char* argv[]){
    trykill(argv[0], argv[1]);
    exit();
}