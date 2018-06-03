/* gcc -Wall -ansi -pthread -lrt 2.c*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include "queue.h"

#define NUM_STUDENTS 3
#define MAX_CHAIRS (int) (NUM_STUDENTS/2)

pthread_t STUDENTS[NUM_STUDENTS];
pthread_t AE;

sem_t SEM_AE_STATE;
sem_t SEM_STUDENTS;

sem_t waitingRoom;
sem_t aeChair;

sem_t SEM_AE_HELPING;

pthread_mutex_t mutex_queue_access;
struct Queue * chairs_queue;
pthread_mutex_t mutex_free_chairs;

int free_chairs = MAX_CHAIRS;

void * ae_func(void* arg){
  int helps_given = 0;

  while(helps_given < NUM_STUDENTS*3){
    int random_time = (rand()%5)+1;
    pthread_mutex_lock(&mutex_queue_access);

    if(chairs_queue->size > 0){
      int next = chairs_queue->front_id;

      pthread_mutex_unlock(&mutex_queue_access);

      printf("AE vai ajudar %d\n", next);

      printf("o AE esta ajduando %d\n", next);
      sleep(random_time);
      printf("o AE terminou de ajudar o %d\n", next);

      helps_given++;
      sem_post(&SEM_AE_HELPING);
    } else {
      pthread_mutex_unlock(&mutex_queue_access);
      printf("AE DORMINDO.\n");
      sem_wait(&SEM_AE_STATE);

      printf("AE ACORDOU.\n");
    }
  }
  pthread_exit((void*) 0);
}

void * student_func(void* arg){
  long thread = (long) arg;
  int helps = 0;

  while(helps < 3){
    int random_time = (rand()%5)+1;

    printf("Estudante %lu esta programando.\n", thread);
    sleep(random_time);
    printf("Estudante %lu esta indo procurar o AE.\n", thread);

    pthread_mutex_lock(&mutex_queue_access);
    printf("saiu do mutex\n");

    if(chairs_queue->size < MAX_CHAIRS){
      push_q(chairs_queue, thread);
      pthread_mutex_unlock(&mutex_queue_access);

      sem_wait(&aeChair);

      pthread_mutex_lock(&mutex_queue_access);
      pop_q(chairs_queue);
      pthread_mutex_unlock(&mutex_queue_access);

      printf("Estudante %lu esta acordando o AE.\n", thread);
      sem_post(&SEM_AE_STATE);
      printf("Estudante %lu esta sendo ajudado.\n", thread);
      sem_wait(&SEM_AE_HELPING);

      printf("Estudante %lu foi ajudado.\n", thread);
      sem_post(&aeChair);
      helps++;
    } else{
      pthread_mutex_unlock(&mutex_queue_access);
      printf("Estudante %lu vai voltar a programar pois a fila esta cheia.\n", thread);
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
  sem_init(&waitingRoom, 0, MAX_CHAIRS);
  sem_init(&aeChair, 0, 1);
  sem_init(&SEM_AE_HELPING, 0, 0);

  pthread_mutex_init(&mutex_free_chairs, NULL);
  pthread_mutex_init(&mutex_queue_access, NULL);
  chairs_queue = create_queue();

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
  pthread_mutex_destroy(&mutex_queue_access);

  clear_queue(chairs_queue);
  free(chairs_queue);

  sem_destroy (&SEM_AE_STATE);
  sem_destroy (&SEM_AE_HELPING);
  sem_destroy (&aeChair);
  sem_destroy (&waitingRoom);

  pthread_exit(NULL);
  return 0;
}
