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

/* Function prototype */
void showbits(unsigned int x);
unsigned char * loadPageFromBack(FILE * file, int id_page);
unsigned char request_data(unsigned int page, unsigned int offset);
void map_addresses(FILE * addresses, FILE * backingstore);

/* Global variables - Easier */
FILE * file_addresses = NULL;
FILE * file_storage = NULL;

struct P_Mem * physical_memory = NULL;
unsigned int page_table[PAGE_COUNT];

unsigned char request_data(unsigned int page, unsigned int offset){
    unsigned int location = page_table[page];

    /* First, check if the data is loaded in memory */
    if(location == -1){
        printf("REQUEST DATA: Pagina %u ainda nao esta na memoria. Buscando...\n", page);
        unsigned char * loaded_data = loadPageFromBack(file_storage, page);
        
        /* Loads and updates page_table */
        page_table[page] = load_page(physical_memory, page, loaded_data);
        location = page_table[page];
        
        printf("REQUEST DATA: Pagina %u carregada no frame %u!\n", page, location);
    }

    /* Translates and get the needed char */
    printf("REQUEST DATA: Pagina %u traduzida no frame %u!\n", page, location);
    return physical_memory->frames[location].data[offset];
}

void showbits(unsigned int x){
    int i;
    for(i=(sizeof(unsigned int) * 4)-1; i>=0; i--)
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

            unsigned char page_char =  request_data(id_page, offset_page);
            printf("MAP ADDRESS - Char: %c - ASCII: %u\n", page_char, (unsigned int)page_char);
        }

        putchar('\n');
    }
}

unsigned char * loadPageFromBack(FILE * file, int id_page){
    if(fseek(file,id_page * FRAME_SIZE, SEEK_SET) != 0){
      printf("LOAD PAGE %d: FSEEK FALHOU!\n", id_page);
      return NULL;
    }

    unsigned char * buffer = (unsigned char * ) malloc(sizeof(unsigned char) * FRAME_SIZE);
    if(fread(buffer, sizeof(unsigned char), FRAME_SIZE, file) == 0){
      printf("LOAD PAGE %d: FREAD FALHOU!\n", id_page);
      return NULL;
    }
    return buffer;
}

int main(int argc, char *argv[]){
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

    /* Reading file and mapping*/
    map_addresses(file_addresses, file_storage);

    /* Closing files */
    fclose(file_addresses);
    fclose(file_storage);
    return 0;
}
