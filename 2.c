/* gcc -Wall -ansi -pthread -lrt 2.c*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define NUM_STUDENTS 3
#define MAX_CHAIRS (int) (NUM_STUDENTS/2)

pthread_t STUDENTS[NUM_STUDENTS];
pthread_t AE;

sem_t SEM_AE_STATE;
sem_t SEM_STUDENTS;

pthread_mutex_t mutex_free_chairs;

int free_chairs = MAX_CHAIRS;

void * ae_func(void* arg){
  int counter = 0;
  while( counter < NUM_STUDENTS*3){
    int random_time = (rand()%5)+1;

    printf("AE DORMINDO\n");
    sem_wait(&SEM_STUDENTS);
    printf("AE ACORDOU\n");
    pthread_mutex_lock(&mutex_free_chairs);
    free_chairs++;
    sem_post(&SEM_AE_STATE);
    pthread_mutex_unlock(&mutex_free_chairs);
    printf("AJUDANDO\n");
    counter++;
    sleep(random_time);
  }
  pthread_exit((void*) 0);
}

void * student_func(void* arg){
  long thread = (long) arg;
  int helps = 0;

  while(helps < 3){
    int random_time = (rand()%5)+1;
    printf("estudante %lu programando\n", thread);

    sleep(random_time);
    printf("estudante %lu procurando AE\n", thread);

    pthread_mutex_lock(&mutex_free_chairs);

    if(free_chairs == MAX_CHAIRS){
      free_chairs--;
      sem_post(&SEM_STUDENTS);
      pthread_mutex_unlock(&mutex_free_chairs);
      printf("estudante %lu acordou AE\n", thread);
      sem_wait(&SEM_AE_STATE);
      printf("%lu: RECEBENDO AJUDA\n", thread);
      helps++;
    }else if(free_chairs > 0 && free_chairs < MAX_CHAIRS){
      free_chairs--;
      sem_post(&SEM_STUDENTS);
      pthread_mutex_unlock(&mutex_free_chairs);
      printf("estudante %lu sentou na cadeira\n", thread);
      sem_wait(&SEM_AE_STATE);
      printf("estudante %lu saiu da cadeira\n", thread);
      printf("%lu: RECEBENDO AJUDA\n", thread);
      helps++;
    }else{
      printf("%d\n", free_chairs);
      printf("estudante %lu voltou a programar (TUDO OCUPADO)\n", thread);
      pthread_mutex_unlock(&mutex_free_chairs);
    }
  }
  printf("%lu: FINALIZANDO AJUDA\n", thread);
  pthread_exit((void*) 0);
};


int main(int argc, char * argv[]){

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  sem_init (&SEM_AE_STATE, 0, 0);
  sem_init (&SEM_STUDENTS, 0, 0);

  pthread_mutex_init(&mutex_free_chairs, NULL);

  pthread_create(&AE, &attr, ae_func, NULL);
  long counter;

  for(counter = 0; counter < NUM_STUDENTS; counter++) {
    pthread_create(&STUDENTS[counter], &attr, student_func, (void *)counter);
  }

  void* status;

  for(counter = 0; counter < NUM_STUDENTS; counter++) {
    pthread_join(STUDENTS[counter], &status);
  }

  printf("esperando terminar\n");
  pthread_join(AE, &status);
  printf("terminou\n");


  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex_free_chairs);
  sem_destroy (&SEM_AE_STATE);
  sem_destroy (&SEM_STUDENTS);
  pthread_exit(NULL);
  return 0;
}
