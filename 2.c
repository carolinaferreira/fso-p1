/* gcc -Wall -ansi -lpthread -lrt 2.c*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define NUM_STUDENTS 10
#define MAX_CHAIRS (int) (NUM_STUDENTS/2)

pthread_t STUDENTS[NUM_STUDENTS];
pthread_t AE;

sem_t SEM_AE_STATE;

int QUEUE_IDS[MAX_CHAIRS];

int queue_size = 0;
pthread_mutex_t mutex_queue_size;

int main(int argc, char * argv[]){
    return 0;
}