/*
    1: gcc -Wall -ansi -lpthread 1.c
    2: ./a.out test1.txt
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define GRID_W 9
#define GRID_H 9

extern int errno;

int SUM_THREAD = 0;
pthread_mutex_t SUM_THREAD_MUTEX;

unsigned short GRID[GRID_H][GRID_W];
pthread_mutex_t GRID_MUTEX[GRID_H][GRID_W];

pthread_t LINE_THREAD[GRID_W];
pthread_t COLUMN_THREAD[GRID_H];

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
            GRID[height][width] = number;
        }
    }
}

void init_mutexes(){
    int height, width;

    for(height = 0; height < GRID_H; height++){
        for(width = 0; width < GRID_W; width++){
            pthread_mutex_init(&GRID_MUTEX[height][width], NULL);
        }
    }
    pthread_mutex_init(&SUM_THREAD_MUTEX, NULL);
}

void destroy_mutexes(){
    int height, width;

    for(height = 0; height < GRID_H; height++){
        for(width = 0; width < GRID_W; width++){
            pthread_mutex_destroy(&GRID_MUTEX[height][width]);
        }
    }
    pthread_mutex_destroy(&SUM_THREAD_MUTEX);
}

int *init_array(int size){
  int *a;
  a = (int*) malloc (size*sizeof(int));

  int counter;
  for(counter = 0; counter < size; counter++){
      a[counter] = 0;
  }
  
  return a;
}

void *check_line(void *arg){
    int *array = init_array(GRID_W+1);
    int counter;
    int sum = 0;
    long line = (long) arg;

    for(counter = 0; counter < GRID_W ; counter++ ){
        pthread_mutex_lock (&GRID_MUTEX[line][counter]);
        unsigned short grid_copy = GRID[line][counter];
        pthread_mutex_unlock (&GRID_MUTEX[line][counter]);

        if(array[grid_copy] == 0){
          sum++;
        }

        array[grid_copy]++;
    }

    if(sum == GRID_W){
      printf("Linha %lu eh valida\n", line);
      pthread_mutex_lock (&SUM_THREAD_MUTEX);
      SUM_THREAD++;
      pthread_mutex_unlock (&SUM_THREAD_MUTEX);
    }else{
      printf("Linha %lu nao eh valida\n", line);
    }

    free(array);
    pthread_exit((void*) 0);
}

void *check_column(void *arg){
    int *array = init_array(GRID_H+1);
    int counter;
    int sum = 0;
    long column = (long) arg;

    for(counter = 0; counter < GRID_H ; counter++ ){
        pthread_mutex_lock (&GRID_MUTEX[counter][column]);
        unsigned short grid_copy = GRID[counter][column];
        pthread_mutex_unlock (&GRID_MUTEX[counter][column]);

        if(array[grid_copy] == 0){
          sum++;
        }

        array[grid_copy]++;
    }

    if(sum == GRID_H){
      printf("Coluna %lu eh valida\n", column);
      pthread_mutex_lock (&SUM_THREAD_MUTEX);
      SUM_THREAD++;
      pthread_mutex_unlock (&SUM_THREAD_MUTEX);
    }else{
      printf("Coluna %lu nao eh valida\n", column);
    }

    free(array);
    pthread_exit((void*) 0);
}

int main(int argc, char * argv[]){
    FILE * file_arg = NULL;

    /* Opening file */
    file_arg = fopen(argv[1], "r+");

    /*printf("%s\n", argv[1]);*/

    if(file_arg != NULL){
        load_grid(file_arg);
        /*print_grid();*/
    } else {
        perror("Aconteceu um erro");
    }

    /* Initializing mutex */
    init_mutexes();

    /*PTHREAD initialization*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    long counter;

    for(counter = 0; counter < GRID_H; counter++) {
      pthread_create(&LINE_THREAD[counter], &attr, check_line, (void *)counter);
    }

    for(counter = 0; counter < GRID_H; counter++) {
      pthread_create(&COLUMN_THREAD[counter], &attr, check_column, (void *)counter);
    }

    void* status;

    for(counter = 0; counter < GRID_H; counter++) {
      pthread_join(LINE_THREAD[counter], &status);
      pthread_join(COLUMN_THREAD[counter], &status);
    }

    printf("\nAQUI ESTA O RESULTADO: %d\n", SUM_THREAD);

    fclose(file_arg);

    pthread_attr_destroy(&attr);
    destroy_mutexes();
    pthread_exit(NULL);

    return 0;
}
