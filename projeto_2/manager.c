/*  1: gcc -Wall -ansi -O2 manager.c
    2: ./a.out addresses.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "physical_mem.h"

#define BACKING_STORAGE_FILE "BACKING_STORE.bin"
#define FRAME_SIZE 256
#define FRAME_COUNT 256
#define BYTE_LIMIT (FRAME_COUNT * FRAME_SIZE)

/* Global variables - Easier */
struct P_Mem * physical_memory = NULL;

void showbits(unsigned int x)
{
    int i; 
    for(i=(sizeof(int)*8)-1; i>=0; i--)
            (x&(1u<<i))?putchar('1'):putchar('0');
    
    printf("\n");
}

void map_addresses(FILE * addresses){
    while(!feof(addresses)){
        unsigned int adr = 0;

        fscanf(addresses, "%u", &adr);
        printf("Lendo endereço do arquivo: %u - ", adr);
        showbits(adr);
        
        int id_page = adr >> 8;
        printf("Id da pagina: %d - ", id_page);
        showbits(id_page);
        
        int offset_page = (adr & 0xFF);
        printf("Offset da pagina: %d - ", offset_page);
        showbits(offset_page);
    }
}


int main(int argc, char *argv[]){
    FILE * file_addresses = NULL;
    FILE * file_storage = NULL;

    /* If there isn't a file as argument */
    if(argc != 2){
        printf("Missing arg!\n");
        return -1;
    }

    /* Opening file */
    file_addresses = fopen(argv[1], "r+");
    file_storage = fopen(BACKING_STORAGE_FILE, "r+");

    /* Checking for opening error */
    if(file_addresses == NULL || file_storage == NULL){
        perror("Aconteceu um erro");
        return -1;
    }

    /* Preparing memories */
    physical_memory = create_p_mem(FRAME_COUNT);

    /* Reading file */
    map_addresses(file_addresses);

    fclose(file_addresses);
    fclose(file_storage);
    return 0;
}