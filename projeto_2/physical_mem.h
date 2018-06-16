#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../projeto_1/queue.h"

#define INVALID_ADDRESS -1

struct Frame {
    unsigned int virtual_address;
    unsigned char * data;
};

struct P_Mem {
    int mem_size;
    struct Queue * available_frames;
    struct Queue * removal_order;
    struct Frame * frames;
};

struct Frame create_frame(int v_address, unsigned char * data){
    struct Frame * new_frame = NULL;

    new_frame = (struct Frame *)malloc(sizeof(struct Frame));

    if(new_frame != NULL){
        new_frame->virtual_address = v_address;
        new_frame->data = data;
    }

    return * new_frame;
}

void initialize_frame_list(struct P_Mem * mem, unsigned int frame_count){
    struct Frame * new_array = NULL;
    new_array = (struct Frame *)malloc(sizeof(struct Frame) * frame_count);

    if(new_array != NULL){
        int counter = 0;
        for(counter = 0; counter < frame_count; counter++){
            new_array[counter] = create_frame(INVALID_ADDRESS, NULL);
            push_q(mem->available_frames, counter);
        }
    }

    mem->frames = new_array;
}

struct P_Mem * create_p_mem(unsigned int frame_count){
    struct P_Mem * new_mem = NULL;

    new_mem = (struct P_Mem *)malloc(sizeof(struct P_Mem));

    if(new_mem != NULL){
        new_mem->mem_size = frame_count;
        new_mem->available_frames = create_queue();
        new_mem->removal_order = create_queue();
        
        initialize_frame_list(new_mem, frame_count);
    }

    return new_mem;
}

/* This function is responsible for adding a page to memory */
/* Output: Int value which the page was loaded */ 
int load_page(struct P_Mem * mem, unsigned int v_address, unsigned char * data){
    int page_position = INVALID_ADDRESS;

    if(mem->available_frames->size > 0){
        /* Read first available index then pop available queue */
        page_position = mem->available_frames->front_id;
        pop_q(mem->available_frames);
    } else {
        /* Remove any page */
        page_position = mem->removal_order->front_id;
        pop_q(mem->removal_order);
    }

    /* Adding new page to removal order */
    push_q(mem->removal_order, page_position);
    
    /* Updating the frame with the new page */
    mem->frames[page_position].data = data;
    mem->frames[page_position].virtual_address = v_address;

    return page_position;
}


