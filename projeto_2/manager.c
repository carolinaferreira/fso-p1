/*  1: gcc -Wall -ansi -O2 manager.c
    2: ./a.out test1.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define FRAME_SIZE 256
#define FRAME_COUNT 256
#define BYTE_LIMIT (FRAME_COUNT * FRAME_SIZE)

void map_adresses(FILE * adresses){
    while(!feof(adresses)){
        int adr = 0;

        fscanf(adresses, "%d", &adr);
        printf("Li: %d\n", adr);
    }
}

int main(int argc, char *argv[]){
    FILE * file_adresses = NULL;

    /* If there isn't a file as argument */
    if(argc != 2){
        printf("Missing arg!\n");
        return -1;
    }

    /* Opening file */
    file_adresses = fopen(argv[1], "r+");

    /* Checking for opening error */
    if(file_adresses == NULL){
        perror("Aconteceu um erro");
        return -1;
    }

    /* Reading file */
    map_adresses(file_adresses);

    fclose(file_adresses);
    return 0;
}