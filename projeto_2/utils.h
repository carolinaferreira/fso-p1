#include <stdio.h>
#include <stdlib.h>

#pragma once

/* This function prints a int in its binary form. 16 bits */
void showbits(unsigned int x){
    int i;
    for(i=(sizeof(unsigned int) * 4)-1; i>=0; i--)
            (x&(1u<<i))?putchar('1'):putchar('0');

    printf("\n");
}

/* This function is resposible for reading a entire page from the backing_store file and returns the page
    char array */
unsigned char * loadPageFromBack(FILE * file, int id_page, int frame_size){
    if(fseek(file,id_page * frame_size, SEEK_SET) != 0){
      printf("LOAD PAGE %d: FSEEK FALHOU!\n", id_page);
      return NULL;
    }

    unsigned char * buffer = (unsigned char * ) malloc(sizeof(unsigned char) * frame_size);
    if(fread(buffer, sizeof(unsigned char), frame_size, file) == 0){
      printf("LOAD PAGE %d: FREAD FALHOU!\n", id_page);
      return NULL;
    }
    return buffer;
}