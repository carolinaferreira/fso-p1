#include <stdlib.h>

struct Node{
    int id;
    struct Node * previous;
    struct Node * next;
};

struct Queue{
    int size, front_id, back_id;
    struct Node * front;
    struct Node * back;
};

/* This function creates a 'empty' node with only the id */
struct Node * create_node(int id){
    struct Node * new_node = (struct Node *)malloc(sizeof(struct Node));
    new_node->previous = NULL;
    new_node->next = NULL;
    new_node->id = id;

    return new_node;
}

/* This function creates a 'empty' queue */
struct Queue * create_queue(){
    struct Queue * new_q = (struct Queue *)malloc(sizeof(struct Queue));
    new_q->front = NULL;
    new_q->back = NULL;

    new_q->size = 0;
    new_q->front_id = 0;
    new_q->back_id = 0;

    return new_q;
}

/* This function adds a node in a queue */
void push_q(struct Queue * queue, int value){
    struct Node * new_node = create_node(value);
    
    if(queue->size <= 0){
        queue->front = new_node;
        queue->back = new_node;

        queue->front_id = value;
        queue->back_id = value;
    } else {
        new_node->previous = queue->back;
        queue->back->next = new_node;
        
        queue->back = new_node;
    }
    
    queue->size += 1;
}

/* This function removes a node from the queue */
void pop_q(struct Queue * queue){
    if(queue->size > 1){
        struct Node * old_front = queue->front;
        
        queue->front = old_front->next;
        queue->front->previous = NULL;
        queue->size -= 1;
        queue->front_id = queue->front->id;

        free(old_front);
    } else {
        free(queue->front);

        queue->size -= 1;
        queue->front = NULL;
        queue->back = NULL;
        queue->front_id = 0;
        queue->back_id = 0;
    }
}

/* This function erases the queue */
void clear_queue(struct Queue * q){
    int counter;
    int max_size = q->size;
    for(counter = 0; counter < max_size; counter++){
        pop_q(q);
    }
}

void print_queue(struct Queue * q){
    int counter = 0;
    struct Node * actual = q->front;
    printf("FILA [frente =  %d]:", q->front_id);

    for(counter = 0; counter < q->size; counter++){
        printf(" [pos %d]: %d", counter, actual->id);
        actual = actual->next;
    }
    printf("\n");
}