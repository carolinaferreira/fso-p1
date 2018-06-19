#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../projeto_1/queue.h"
#include "utils.h"
#include "physical_mem.h"

#define FRAME_SIZE 256
#define FRAME_COUNT 256

/* Data definitions */
struct Pair {
  int page;
  int frame;
};

struct TLB {
  unsigned int tlb_size, total, misses, hits;
  int entries_count;

  unsigned int * page_table;
  struct P_Mem * mem;
  FILE * storage;

  struct Pair * entries;
  struct Queue * removal_order;
};

/* Functions prototypes */
struct Pair * initialize_pair_entries(unsigned int tlb_size);
struct TLB * create_tlb(struct P_Mem *  mem, unsigned int * page_table, FILE * storage, unsigned int tlb_size);
unsigned char tlb_request(struct TLB * tlb, unsigned int page, unsigned int offset);
void push_tlb(struct TLB * tlb, unsigned int page, unsigned int frame);
void get_statistics(struct TLB * tlb);

/* This function creates a 'blank' pair data type */
struct Pair create_pair(int page, int frame){
    struct Pair * new_pair = NULL;

    new_pair = (struct Pair *)malloc(sizeof(struct Pair));

    if(new_pair != NULL){
        new_pair->page = page;
        new_pair->frame = frame;
    } else {
      printf("ERRO: Falha ao criar novo par! Pg: %u, Frame: %u.", page, frame);
    }

    return * new_pair;
}

/* This function creates an array of TLB entries (Pair data type) and returns the address */
struct Pair * initialize_pair_entries(unsigned int tlb_size){
    struct Pair * new_array = NULL;
    new_array = (struct Pair *)malloc(sizeof(struct Pair) * tlb_size);

    if(new_array != NULL){
        unsigned int counter = 0;
        for(counter = 0; counter < tlb_size; counter++){
            new_array[counter] = create_pair(INVALID_ADDRESS, INVALID_ADDRESS);
        }
    }

    return new_array;
}

/* This function creates a 'blank' TLB */
struct TLB * create_tlb(struct P_Mem *  mem, unsigned int * page_table, FILE * storage, unsigned int tlb_size){
    struct TLB * new_tlb = NULL;
    new_tlb = (struct TLB *)malloc(sizeof(struct TLB));

    if(new_tlb != NULL){

      new_tlb->total = 0;
      new_tlb->misses = 0;
      new_tlb->hits = 0;
      new_tlb->entries_count = 0;
      new_tlb->tlb_size = tlb_size;

      new_tlb->mem = mem;
      new_tlb->page_table = page_table;
      new_tlb->storage = storage;

      new_tlb->removal_order =  create_queue();
      new_tlb->entries = initialize_pair_entries(tlb_size);
    }

    return new_tlb;
}

/* This functions search for an address in TLB. If not found, loads the data in the main memory. Then,
   the function returns the requested char value */
unsigned char tlb_request(struct TLB * tlb, unsigned int page, unsigned int offset){
  int location = -1;

  /* Adding total request number */
  tlb->total++;

  /* Linear search in TLB entries - Not sorted */
  unsigned int counter = 0;
  for(counter = 0; counter < tlb->tlb_size; counter++){
    if((unsigned int)tlb->entries[counter].page == page){
      tlb->hits++;
      printf("TLB REQ: Pagina %u esta no TLB!\n", page);
      location = tlb->entries[counter].frame;
      break;
    }
  }

  /* If not in TLB, check if it's loaded in page_table */
  if(location == -1 && (int)tlb->page_table[page] != -1){
    tlb->misses++;
    printf("TLB REQ: Pagina %u nao esta no TLB! - Encontrado na page_table com frame %u!\n",
      page, tlb->page_table[page]);

    /* Updates the tlb */
    location = tlb->page_table[page];
    push_tlb(tlb, page, location);
    printf("TLB REQ: Pagina %u foi carregada no TLB!\n", page);
  } else if(location == -1 && (int)tlb->page_table[page] == -1){
    tlb->misses++;
    printf("TLB REQ: Pagina %u nao esta no TLB! - Nao encontrado na page_table. Necessario carregar.\n",
      page);

    /* Page must be loaded */
    unsigned char * loaded_data = loadPageFromBack(tlb->storage, page, FRAME_SIZE);

    /* Loads and updates page_table */
    tlb->page_table[page] = load_page(tlb->mem, page, loaded_data);

    /* Updates the tlb */
    location = tlb->page_table[page];
    push_tlb(tlb, page, location);
    printf("TLB REQ: Pagina %u foi carregada no TLB!\n", page);
  }

  /* Return the char */
  unsigned frame_begin_byte = location * FRAME_COUNT;
  unsigned frame_end_byte = frame_begin_byte + FRAME_SIZE;
  printf("TLB REQ: Pagina %u foi traduzida para o frame %u (%d bytes - %d bytes)!\n",
    page, location, frame_begin_byte, frame_end_byte);
  return tlb->mem->frames[location].data[offset];
}

/* This function pushes an address into the TLB, even of TLB is full */
void push_tlb(struct TLB * tlb, unsigned int page, unsigned int frame){
  unsigned int new_index = -1;

  if((unsigned int)tlb->entries_count < tlb->tlb_size){
    /* Still not full. Just add */
    new_index = tlb->entries_count;
  } else {
    /* Removes one and add */
    new_index = tlb->removal_order->front_id;
    pop_q(tlb->removal_order);
  }
    /* Updates old frame with new info */
    tlb->entries[new_index].page = page;
    tlb->entries[new_index].frame = frame;
    tlb->entries_count++;
    push_q(tlb->removal_order, new_index);
}

/* This function prints the TLB hit/miss statistics */
void get_statistics(struct TLB * tlb){
  printf("\nTotal de lookup no TLB: %u\n", tlb->total);

  double miss_rate = ((double)tlb->misses / (double)tlb->total)*100.0;
  double hit_rate = ((double)tlb->hits / (double)tlb->total)*100.0;
  printf("Taxa de acerto de paginas: %.2lf%%\n", miss_rate);
  printf("Taxa de erro de paginas: %.2lf%%\n", hit_rate);
}
