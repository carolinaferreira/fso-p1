/* Command line: gcc -Wall -ansi -lpthread 1.c */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define GRID_W 9
#define GRID_H 9

extern int errno;

unsigned short GRID[GRID_H][GRID_W];
pthread_mutex_t GRID_MUTEX[GRID_H][GRID_W];

void print_grid(){
    int height, width;

    for(height = 0; height < GRID_H; height++){
        for(width = 0; width < GRID_W; width++){
            printf("%d ", GRID[height][width]);
        }

        printf("\n");
    }
}

void load_grid(FILE * file){
    int height, width;

    for(height = 0; height < GRID_H; height++){
        for(width = 0; width < GRID_W; width++){
            int number = 0;
            fscanf(file, "%d", &number);
            printf("Reading: %d\n", number);
            GRID[height][width] = number;
        }
    }
}

int main(int argc, char * argv[]){
    FILE * file_arg = NULL;
    file_arg = fopen(argv[1], "r+");

    printf("%s\n", argv[1]);

    if(file_arg != NULL){
        load_grid(file_arg);
        print_grid();
    } else {
        perror("Aconteceu um erro");
    }

    fclose(file_arg);
    return 0;
}