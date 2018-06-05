/* gcc -Wall -ansi -pthread -lrt -lm -std=c99 2.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <pthread.h>
#include <semaphore.h>

#include "queue.h"

#define NUM_STUDENTS 45
#define MAX_HELPS 3
#define CHAIRS (NUM_STUDENTS / 2.0)
#define AE_PLANNED_HELPS (NUM_STUDENTS * MAX_HELPS)
#define MAX_SLEEP_TIME 2

int MAX_CHAIRS = 0;

pthread_t STUDENTS[NUM_STUDENTS];
pthread_t AE;

sem_t SEM_AE_HELPING;
sem_t SEM_AE_READY;
sem_t SEM_STUDENTS;
sem_t SEM_STUDENT_LEAVING;
sem_t SEM_STUDENT_READY;
sem_t aeChair;


pthread_mutex_t mutex_queue_access;
struct Queue * chairs_queue;

void * ae_func(void* arg){
  int helps_given = 0;

  while(helps_given < AE_PLANNED_HELPS){
    int random_time = (rand() % MAX_SLEEP_TIME) + 1;
    
    /* Problems with developing a better solution for 'sleep' */
    printf("AE: Dormindo ou esperando...\n");
    sem_wait(&SEM_STUDENTS);
    printf("AE: Acordou!\n");

    printf("AE: Indo conferir a fila.\n");
    pthread_mutex_lock(&mutex_queue_access);
    int actual_queue_size = chairs_queue->size;

    if(actual_queue_size > 0){
      /* Calling the first student in the queue */
      int actual_student = chairs_queue->front_id;
      printf("AE: Antes de chamar -> ");
      print_queue(chairs_queue);
      pop_q(chairs_queue);
      printf("AE: Chamou o estudante %d.\n", actual_student);
      printf("AE: Depois de chamar -> ");
      print_queue(chairs_queue);
      pthread_mutex_unlock(&mutex_queue_access);
      
      /* Waiting for student */
      sem_post(&SEM_AE_READY);
      sem_wait(&SEM_STUDENT_READY);

      /* Helping student */
      printf("AE: Ajudando o estudante %d.\n", actual_student);
      sleep(random_time);

      /* Finishing help */
      sem_post(&SEM_AE_HELPING);
      printf("AE: Terminou de ajudar o estudante %d.\n", actual_student);
      helps_given++;

      /* Waiting for student to leave room */
      sem_wait(&SEM_STUDENT_LEAVING);    
    } else {
      pthread_mutex_unlock(&mutex_queue_access);
    }
  }

  printf("AE: Todas as ajudas planejadas foram dadas. Indo embora...\n");
  pthread_exit((void*) 0);
}

void * student_func(void* arg){
  long thread = (long) arg;
  int helps = 0;

  while(helps < MAX_HELPS){
    int random_time = (rand() % MAX_SLEEP_TIME) + 1;

    printf("Estudante %lu: Programando.\n", thread);
    sleep(random_time);
    printf("Estudante %lu: Indo procurar o AE.\n", thread);

    pthread_mutex_lock(&mutex_queue_access);
    if(chairs_queue->size < MAX_CHAIRS){
      /* Entering in the line */
      printf("Estudante %lu: Entrando na fila do AE.\n", thread);
      push_q(chairs_queue, thread);
      int actual_front = chairs_queue->front_id;
      pthread_mutex_unlock(&mutex_queue_access);

      /* Waiting for its turn */
      printf("Estudante %lu: Esperando sua vez na fila.\n", thread);
      while(actual_front != thread){
        pthread_mutex_lock(&mutex_queue_access);
        actual_front = chairs_queue->front_id;
        pthread_mutex_unlock(&mutex_queue_access);
      }

      /* Waiting for the AE chair to become free */
      printf("Estudante %lu: Esperando a cadeira do AE ficar vazia.\n", thread);
      sem_wait(&aeChair);

      /* If the AE is sleeping, wakes him up */
      printf("Estudante %lu: Avisando o AE que quer tirar duvidas.\n", thread);
      sem_post(&SEM_STUDENTS);

      /* Waiting for AE being ready to help */
      printf("Estudante %lu: Esperando ser ajudado pelo AE.\n", thread);
      sem_post(&SEM_STUDENT_READY);
      sem_wait(&SEM_AE_READY);

      /* Waiting for AE finishing helping */
      printf("Estudante %lu: Sendo ajudado pelo AE.\n", thread);
      sem_wait(&SEM_AE_HELPING);

      /* Leaving AE chair */
      printf("Estudante %lu: Saindo da cadeira do AE.\n", thread);
      sem_post(&aeChair);
      helps++;

      /* Leaving AE */
      sem_post(&SEM_STUDENT_LEAVING);
    } else {
      printf("Estudante %lu: Voltou a programar pois a fila esta cheia.\n", thread);
      pthread_mutex_unlock(&mutex_queue_access);
    }
  }

  printf("Estudante %lu: Tirou todas as duvidas. Indo embora.\n", thread);
  pthread_exit((void*) 0);
};


int main(int argc, char * argv[]){
  /* Pthread basic info initialization */
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* Semaphores initialization */
  sem_init (&SEM_AE_READY, 0, 0);
  sem_init(&aeChair, 0, 1);
  sem_init(&SEM_AE_HELPING, 0, 0);
  sem_init(&SEM_STUDENTS, 0, 0);
  sem_init(&SEM_STUDENT_LEAVING, 0, 0);
  sem_init(&SEM_STUDENT_READY, 0, 0);

  /* Random seed config */
  srand(time(NULL));
  MAX_CHAIRS = (int) round(CHAIRS);
  printf("MAIN: MAX_CHAIRS %d\n", MAX_CHAIRS);

  /* Queue initialization */
  pthread_mutex_init(&mutex_queue_access, NULL);
  chairs_queue = create_queue();

  /* Creating pthreads - AE and Students */
  pthread_create(&AE, &attr, ae_func, NULL);
  long counter;

  for(counter = 0; counter < NUM_STUDENTS; counter++) {
    pthread_create(&STUDENTS[counter], &attr, student_func, (void *)counter);
  }

  /* Finishing Students and AE */
  void* status;
  for(counter = 0; counter < NUM_STUDENTS; counter++) {
    pthread_join(STUDENTS[counter], &status);
  }

  printf("MAIN: Todos estudantes foram embora.\n");
  pthread_join(AE, &status);
  printf("MAIN: AE foi embora.\n");

  /* Finishing pthreads and semaphores variables */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex_queue_access);

  sem_destroy (&SEM_AE_READY);
  sem_destroy (&SEM_AE_HELPING);
  sem_destroy (&SEM_STUDENT_LEAVING);
  sem_destroy (&SEM_STUDENTS);
  sem_destroy (&SEM_STUDENT_READY);
  sem_destroy (&aeChair);

  /* Cleaning queue from memory */
  clear_queue(chairs_queue);
  free(chairs_queue);

  /* END */
  pthread_exit(NULL);
  return 0;
}
