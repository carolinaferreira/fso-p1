#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../projeto_1/queue.h"
#include "physical_mem.h"

struct Pair {
  int page;
  int frame;
};

struct TLB {
  unsigned int total, misses, hits;
  int entries_count, tlb_size;

  struct P_Mem * mem;
  struct Pair * entries;
  struct Queue * removal_order;
};

void initialize_pair_entries(unsigned int tlb_size, struct TLB * tlb);
struct TLB * create_tlb(unsigned int tlb_size, struct P_Mem *  mem);
unsigned char tlb_request(struct TLB * tlb, unsigned int page, unsigned int offset);
int table_lookup(struct TLB * tlb, unsigned int page);

struct Pair create_pair(int page, int frame){
    struct Pair * new_pair = NULL;

    new_pair = (struct Pair *)malloc(sizeof(struct Pair));

    if(new_pair != NULL){
        new_pair->page = page;
        new_pair->frame = frame;
    }

    return * new_pair;
}


void initialize_pair_entries(unsigned int tlb_size, struct TLB * tlb){
    struct Pair * new_pair = NULL;
    new_pair = (struct Pair *)malloc(sizeof(struct Pair) * tlb_size);

    if(new_pair != NULL){
        int counter = 0;
        for(counter = 0; counter < tlb_size; counter++){
            new_pair[counter] = create_pair(-1, -1);
            push_q(tlb->removal_order, counter);
        }
    }

    tlb->entries = new_pair;
}

struct TLB * create_tlb(unsigned int tlb_size, struct P_Mem *  mem){
    struct TLB * new_tlb = NULL;

    new_tlb = (struct TLB *)malloc(sizeof(struct TLB));

    if(new_tlb != NULL){

      new_tlb->total = 0;
      new_tlb->misses = 0;
      new_tlb->hits = 0;
      new_tlb->tlb_size = tlb_size;

      new_tlb->mem = mem;
      initialize_pair_entries(tlb_size, new_tlb);
      new_tlb->removal_order =  create_queue();
    }

    return new_tlb;
}

/* Return character*/
unsigned char tlb_request(struct TLB * tlb, unsigned int page, unsigned int offset){
  int frame = table_lookup(tlb, page);

  return tlb->mem->frames[frame].data[offset];
}

/* Return position*/
int table_lookup(struct TLB * tlb, unsigned int page){
  int counter = 0;
  int location = -1;

  for(counter = 0 ; counter < tlb->tlb_size; counter++){
    if(tlb->entries[counter].page == page){
      break;
    }
  }

  if(location == -1){
    int remove = tlb->removal_order->front_id;

    pop_q(tlb->removal_order);
    tlb->entries[remove].page = page;
    push_q(tlb->removal_order, remove);
  }

  return tlb->entries[counter].frame;
}
