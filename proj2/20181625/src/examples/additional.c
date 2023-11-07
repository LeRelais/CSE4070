#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main(int argc, char *argv[]){
    int numArray[5];

    for(int i = 1; i < argc; i++){
        numArray[i-1] = atoi(argv[i]);
    }
    
    int fiboResult = fibo(numArray[0]);
    int maxFour = maxOfFour(numArray[0], numArray[1], numArray[2], numArray[3]);

    printf("%d %d\n", fiboResult, maxFour);
    return EXIT_SUCCESS;
}