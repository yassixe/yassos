//the idea is to create way to
//create a memory allocator for 4ko pages
//i will make a 2-bit map and devide the a the space between 16megs and 48megs


#include "common.h"
//i need 2 bits per 4k,
//number of total 4ks is end-start / 4k
// so i need 2*number of total 4ks
//so in bytes it is : 2*number of total 4ks / 8

//free = 0b00
//not free 01,10,11
//when ask for alocating 2blocks,
//is shoould find two consecutive 00 and make the 01
//for the next allocation of two blocks it should find two consectuve and make them 10
//for the next time find consectuve zeros and make the 11
//free block two is should make it zeros
//i should think more about how to implement that

//TODO : change this enum to define, i use enum for debugging purpose




enum{
    start = 16*(1 << 20),
    end   = 48*(1 << 20),
    bitmap_size = 2 * ((end - start) / (1 << 12)) / 8
};

//32 megs to map.

uint8_t* bitmap[bitmap_size]__attribute__((aligned(8)))={0};
            


//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//                      i think doing it with linked list would be much better
#define NULL (void*)0

#define true  1
#define false 0
#define MEGA  (1 << 20)


typedef struct mem_slice{
    struct mem_slice* prev;
    struct mem_slice* next;
    uint32_t size;
    uint8_t  is_free;
}__attribute__((packed))mem_slice;

mem_slice* memory_head;

void __init__(){
    memory_head = (mem_slice*) start;
    memory_head->is_free = true;
    memory_head->size = end - start;
    memory_head->prev=NULL;
    memory_head->next=NULL;
}


void* k_malloc(uint32_t size_in_bytes){
    uint32_t size = size_in_bytes + sizeof(mem_slice);
    mem_slice* tmp = memory_head;
    while(tmp->is_free == false || tmp->size < size){
        tmp = tmp->next;
        if(tmp == NULL){
            print_s("not enough memory\n");
            return NULL;
        }
    }
    //
    uint32_t p_size = tmp->size;
    mem_slice* p_prev=tmp->prev;
    mem_slice* p_next=tmp->next;
    mem_slice* next = ((uint32_t)tmp+size >= end || tmp->size == size) ? NULL : (mem_slice*)((uint32_t)tmp+size);

    //tmp
    tmp->is_free=false;
    tmp->size = size;
    tmp->next = next;
    tmp->prev = p_prev;

    if(next){
        next->is_free = true;
        next->size = p_size - size;
        next->prev = tmp;
        next->next = p_next;
    }
    if(p_next && next){
        p_next->prev = next;
    }
    //im not sure if this correct
    return (void*)tmp + 13;
}




void free(void* slice){
    //add asserts to make sure paramss are ok.
    //assert que elle est vraiement not free ...

    uint8_t state_next,state_prev = 0;
    mem_slice* f_slice = slice;
    mem_slice* next=f_slice->next;
    mem_slice* prev=f_slice->prev;
    uint32_t size = f_slice->size;

    f_slice->is_free = true;

    if(next && next->is_free == true) state_next = 1;
    if(prev && prev->is_free == true) state_prev = 1;

    if(state_next && state_prev){
        prev->next = next->next;
        prev->size += size + next->size;
        if(next->next) next->next->prev = prev;
    }
    else if(state_next){
        f_slice->next = next->next;
        f_slice->size +=  next->size;
        if(next->next) next->next->prev = f_slice;
    }
    else if(state_prev){
        prev->next = f_slice->next;
        prev->size += f_slice->size;
        if(f_slice->next) f_slice->next->prev = prev;
    }
}



void allocate(){
    mem_slice* tab[10]={0};
    for(int i=0;i<10;i++){
        tab[i]=k_malloc(7);
    };
    (void)tab;
    while(1);
}
