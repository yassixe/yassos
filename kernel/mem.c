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
            


