/*  1: gcc -Wall -ansi -O2 manager.c
    2: c
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "physical_mem.h"

#define BACKING_STORAGE_FILE "BACKING_STORE.bin"
#define FRAME_SIZE 256
#define FRAME_COUNT 256
#define BYTE_LIMIT (FRAME_COUNT * FRAME_SIZE)
#define PAGE_COUNT 256
#define TLB_COUNT 16

/* Global variables - Easier */
struct P_Mem * physical_memory = NULL;
unsigned char * loadPageFromBack(FILE * file, int id_page, int offset_page);
unsigned int page_table[PAGE_COUNT];


void showbits(unsigned int x) {
    int i;
    for(i=(sizeof(int)*8)-1; i>=0; i--)
            (x&(1u<<i))?putchar('1'):putchar('0');

    printf("\n");
}

void map_addresses(FILE * addresses, FILE * backingstore){
    while(!feof(addresses)){
        unsigned int adr = 0;

        if (fscanf(addresses, "%u", &adr) != 0){
          printf("Lendo endereço do arquivo: %u - ", adr);
          showbits(adr);

          int id_page = adr >> 8;
          printf("Id da pagina: %d - ", id_page);
          showbits(id_page);

          int offset_page = (adr & 0xFF);
          printf("Offset da pagina: %d - ", offset_page);
          showbits(offset_page);
          loadPageFromBack(backingstore, id_page, offset_page);
        }
    }
}

unsigned char * loadPageFromBack(FILE * file, int id_page, int offset_page){
    if(fseek(file,id_page * FRAME_SIZE, SEEK_SET) != 0){
      printf("FSEEK FALHOU\n");
      return NULL;
    }

    unsigned char * buffer = (unsigned char * ) malloc(sizeof(unsigned char) * FRAME_SIZE);
    if(fread(buffer, sizeof(unsigned char), FRAME_SIZE, file) == 0){
      printf("FREAD FALHOU\n");
      return NULL;
    }

    printf("char: %c, valor: %u\n", buffer[offset_page], (unsigned int) buffer[offset_page]);
    return buffer;
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
    file_storage = fopen(BACKING_STORAGE_FILE, "rb");

    /* Checking for opening error */
    if(file_addresses == NULL || file_storage == NULL){
        perror("Aconteceu um erro");
        return -1;
    }

    /* Preparing memories */
    physical_memory = create_p_mem(FRAME_COUNT);

    memset(page_table, -1, PAGE_COUNT * sizeof(unsigned int));

    /* Reading file */
    map_addresses(file_addresses, file_storage);

    fclose(file_addresses);
    fclose(file_storage);
    return 0;
}
