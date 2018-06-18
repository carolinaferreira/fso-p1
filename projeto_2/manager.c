/*  1: gcc -Wall -ansi -O2 manager.c
    2: ./a.out addresses.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "physical_mem.h"
#include "utils.h"
#include "tlb.h"

#define BACKING_STORAGE_FILE "BACKING_STORE.bin"
#define FRAME_SIZE 256
#define FRAME_COUNT 256
#define PAGE_COUNT 256
#define TLB_COUNT 16

/* Function prototype */
unsigned char direct_request(unsigned int page, unsigned int offset);
void map_addresses(FILE * addresses, FILE * backingstore);

/* Global variables - Easier */
FILE * file_addresses = NULL;
FILE * file_storage = NULL;

struct P_Mem * physical_memory = NULL;
struct TLB *  tlb =  NULL;
unsigned int page_table[PAGE_COUNT];

unsigned char direct_request(unsigned int page, unsigned int offset){
    unsigned int location = page_table[page];

    /* First, check if the data is loaded in memory */
    if(location == -1){
        printf("REQUEST DATA: Pagina %u ainda nao esta na memoria. Buscando...\n", page);
        unsigned char * loaded_data = loadPageFromBack(file_storage, page, FRAME_SIZE);

        /* Loads and updates page_table */
        page_table[page] = load_page(physical_memory, page, loaded_data);
        location = page_table[page];

        printf("REQUEST DATA: Pagina %u carregada no frame %u!\n", page, location);
    }

    /* Translates and get the needed char */
    printf("REQUEST DATA: Pagina %u traduzida no frame %u!\n", page, location);
    printf("REQUEST DATA: Endereço %p\n", &physical_memory->frames[location].data[offset]);
    return physical_memory->frames[location].data[offset];
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

            /* unsigned char page_char =  direct_request(id_page, offset_page);
            printf("MAP ADDRESS - Char: %c - ASCII: %u\n", page_char, (unsigned int)page_char);*/

            unsigned char tlb_page_char =  tlb_request(tlb, id_page, offset_page);
            printf("Char: %c - ASCII: %u\n", tlb_page_char, (unsigned int)tlb_page_char);
        }

        putchar('\n');
    }
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
    tlb = create_tlb(physical_memory, page_table, file_storage, TLB_COUNT);
    memset(page_table, -1, PAGE_COUNT * sizeof(unsigned int));


    /* Reading file and mapping*/
    map_addresses(file_addresses, file_storage);

    /* Closing files */
    fclose(file_addresses);
    fclose(file_storage);

    /* Statistics */
    get_statistics(tlb);
    
    return 0;
}
