// the idea is to create way to
// create a memory allocator for 4ko pages
// i will make a 2-bit map and devide the a the space between 16megs and 48megs

#include "common.h"
// i need 2 bits per 4k,
// number of total 4ks is end-start / 4k
//  so i need 2*number of total 4ks
// so in bytes it is : 2*number of total 4ks / 8

// free = 0b00
// not free 01,10,11
// when ask for alocating 2blocks,
// is shoould find two consecutive 00 and make the 01
// for the next allocation of two blocks it should find two consectuve and make them 10
// for the next time find consectuve zeros and make the 11
// free block two is should make it zeros
// i should think more about how to implement that

// TODO : change this enum to define, i use enum for debugging purpose

extern uint32_t pos_x;
extern uint32_t pos_y;

enum
{
    start = 16 * (1 << 20),
    end = 17 * (1 << 20),
    bitmap_size = 2 * ((end - start) / (1 << 12)) / 8
};

// 32 megs to map.

uint8_t *bitmap[bitmap_size] __attribute__((aligned(8))) = {0};

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//                      i think doing it with linked list would be much better
#define NULL (void *)0
#define RDM 0xA7
#define true 1
#define false 0
#define MEGA (1 << 20)

uint8_t hash(void *ptr, uint32_t size)
{
    return (uint8_t)((uint32_t)ptr ^ (size / 2 + RDM));
}

typedef struct mem_slice
{
    struct mem_slice *prev;
    struct mem_slice *next;
    uint32_t size;
    uint8_t is_free;
    uint8_t hash;
} __attribute__((packed)) mem_slice;

mem_slice *memory_head;

void __init__()
{
    memory_head = (mem_slice *)start;
    memory_head->is_free = true;
    memory_head->size = end - start;
    memory_head->prev = NULL;
    memory_head->next = NULL;
    memory_head->hash = hash(memory_head, memory_head->size);
}

void *k_malloc(uint32_t size_in_bytes)
{
    if (size_in_bytes == 0)
        return 0;
    uint32_t size = size_in_bytes + sizeof(mem_slice);
    mem_slice *tmp = memory_head;
    while (!((tmp->is_free == true) && (tmp->size > (size + sizeof(mem_slice)) || tmp->size == size)))
    {
        tmp = tmp->next;
        if (tmp == NULL)
        {
            print_s("not enough memory\n");
            return NULL;
        }
    }

    // need to figure out how the loader work
    uint32_t p_size = tmp->size;
    //mem_slice *p_prev = tmp->prev;
    mem_slice *p_next = tmp->next;

    // note the line 92 contains an error discorvered by test 8
    // i should study more cases.

    /*case 1 : tmp->size > (size + sizeof(mem_slice))*/
    if(tmp->size > (size + sizeof(mem_slice))){//p_size = tmp->size
        if(p_next == NULL){/*si le next est egale a 0 (la fin)*/
            /*==========================next*/
            mem_slice *next = NULL;
            next = (mem_slice*)((uint32_t)tmp + size);
            next->prev = tmp;
            next->next = p_next; //NULL
            next->size = p_size - size;//
            assert(next->size > sizeof(mem_slice));
            next->is_free = true; //no need to calculate the hash here.
            
            /*==========================tmp*/
            tmp->next = next;
            tmp->size = size;
            tmp->is_free =false;
            tmp->hash=hash(tmp,tmp->size);
            /*==========================*/

        }else{//p_next is not null.
            mem_slice *next = NULL;
            next = (mem_slice*)((uint32_t)tmp + size);
            /*==========================p_next*/
            assert(p_next->is_free == false);
            p_next->prev = next;
            /*==========================next*/
            next->prev = tmp;
            next->next = p_next;
            next->is_free = true;
            next->size =  p_size - size;
            assert(next->size > sizeof(mem_slice));
            /*==========================tmp*/
            tmp->next = next;
            tmp->is_free = false;
            tmp->size = size;
            tmp->hash=hash(tmp,tmp->size); 
        }
    }else{/*case 1 : tmp->size == size*/
        tmp->is_free = false;
        tmp->hash = hash(tmp,size); // try the case 2 alloc cover all the size and see it the linked list is correct.
        if(p_next) assert(p_next->is_free == false);
    }

    return (void *)tmp + sizeof(mem_slice);


    // mem_slice* next = ((uint32_t)tmp+size >= end || tmp->size == size) ? NULL : (mem_slice*)((uint32_t)tmp+size);
    // mem_slice *next = NULL;
    // if (tmp->size >= (size + 15))
    //     next = (mem_slice *)((uint32_t)tmp + size);
    // if (tmp->size == size)
    //     next = p_next;

    // // tmp
    // tmp->is_free = false;
    // tmp->size = size;
    // tmp->next = next;
    // tmp->prev = p_prev;

    // if (next && next != p_next)
    // {
    //     next->is_free = true;
    //     next->size = p_size - size;
    //     next->prev = tmp;
    //     next->next = p_next;
    //     // next->hash=hash(next,next->size);
    // }
    // if (p_next && next)
    // {
    //     if (p_next != next)
    //         p_next->prev = next;
    // }
    // // im not sure if this correct
    // tmp->hash = hash(tmp, tmp->size);
    // return (void *)tmp + 14;
}

void k_free(void *ptr)
{
    // add asserts to make sure paramss are ok.
    // assert que elle est vraiement not free ...
    mem_slice *f_slice = (mem_slice *)((uint32_t)ptr - sizeof(mem_slice));
    assert(f_slice->hash == hash(f_slice, f_slice->size));
    assert(f_slice->is_free == false);


    //here start the code rewrite//





    uint8_t state_next = 0, state_prev = 0;
    mem_slice *next = f_slice->next;
    mem_slice *prev = f_slice->prev;
    uint32_t size = f_slice->size;

    f_slice->is_free = true;

    if (next && next->is_free == true)
        state_next = 1;
    if (prev && prev->is_free == true)
        state_prev = 1;

    if (state_next && state_prev)
    {
        prev->next = next->next;
        prev->size += size + next->size;
        if (next->next)
            next->next->prev = prev;
    }
    else if (state_next)
    {
        f_slice->next = next->next;
        f_slice->size += next->size;
        if (next->next)
            next->next->prev = f_slice;
    }
    else if (state_prev)
    {
        prev->next = f_slice->next;
        prev->size += f_slice->size;
        if (f_slice->next)
            f_slice->next->prev = prev;
    }
}

void allocate()
{
    void *tab[10] = {0};
    for (int i = 0; i < 10; i++)
    {
        tab[i] = k_malloc(7);
    };
    for (int i = 0; i < 10; i++)
    {
        k_free(tab[i]);
    };
    while (1)
        ;
}

void allocator_test_0()
{
    assert(k_malloc(0) == NULL);
}

void allocator_test_1()
{
    void *ptr = k_malloc(1);
    assert(ptr != NULL);
    k_free(ptr);
}
void allocator_test_2()
{
    void *ptr = k_malloc(65);
    assert(ptr != NULL);
    k_free(ptr);
}
void allocator_test_3()
{
    void *ptr = k_malloc(4096);
    assert(ptr != NULL);
    k_free(ptr);
}

void allocator_test_4()
{
    void *tab[100] = {0};

    for (int i = 0; i < 100; i++)
    {
        tab[i] = k_malloc(64);
        assert(tab[i] != NULL);
    }
    for (int i = 0; i < 100; i++)
    {
        k_free(tab[i]);
    }
}

void allocator_test_5()
{
    void *tab[100] = {0};
    for (int i = 0; i < 100; i++)
    {
        tab[i] = k_malloc(118);
        assert(tab[i] != NULL);
    }
    for (int i = 0; i < 100; i++)
    {
        k_free(tab[i]);
    }
}

void allocator_test_6()
{
    void *tab[100] = {0};
    for (int i = 0; i < 100; i++)
    {
        tab[i] = k_malloc(118);
        assert(tab[i] != NULL);
    }

    for (int i = 0; i < 100; i += 2)
    {
        k_free(tab[i]);
    }
    for (int i = 1; i < 100; i += 2)
    {
        k_free(tab[i]);
    }
}

void print_linked()
{
    clear_screen();
    mem_slice *tmp = memory_head;
    while (tmp != NULL)
    {
        k_print("ptr:0x%x,size:0x%x,next:0x%x,prev:0x%x,next:0x%x,isfree:0x0000000%x\n", (uint32_t)tmp,
                (uint32_t)tmp->size,
                (uint32_t)tmp->next,
                (uint32_t)tmp->prev,
                (uint32_t)tmp->next,
                (uint8_t)tmp->is_free);
        tmp = tmp->next;
    }
    k_print("-----------------------------------------------------------------------\n");
}

void allocator_test_7()
{
    void *tab[100] = {0};
    int size = 6;
    for (int i = 0; i < 100; i++)
    {
        tab[i] = k_malloc(size);
        assert(tab[i] != NULL);
    }

    for (int i = 0; i < 100; i++)
    {
        if (i % 3 == 1 || i % 3 == 2)
            k_free(tab[i]);
    }

    mem_slice *ptr = memory_head;
    int i = 0;
    while (ptr->next != NULL)
    {
        if (i == 0)
        {
            assert(ptr->size == (size + 14));
            i++;
        }
        else
        {
            assert(ptr->size == (2 * (size + 14)));
            i--;
        }
        ptr = ptr->next;
    }
    for (int i = 0; i < 100; i++)
    {
        if (!(i % 3 == 1 || i % 3 == 2))
            k_free(tab[i]);
    }
}

void print_size_mem()
{
    mem_slice *tmp = memory_head;
    uint32_t result = 0;
    while (tmp)
    {
        result += tmp->size;
        tmp = tmp->next;
    }
    k_print("the size is : 0x%x\n", result);
}
void allocator_test_8()
{
    void *tab[100] = {0};
    uint32_t size = 119;
    for (int i = 0; i < 8; i++)
    {
        tab[i] = k_malloc(size);
        assert(tab[i] != NULL);
    }
    for (int i = 0; i < 8; i += 2)
    {
        k_free(tab[i]);
    }
    for (int i = 0; i < 8; i++)
    {
        mem_slice *tmp = tab[i] - 14;
        if (i % 2 == 0)
            assert((tmp->size == size + 14) && (tmp->is_free == true));
        if (i % 2 == 1)
            assert((tmp->size == size + 14) && (tmp->is_free == false));
    }
    for (int i = 0; i < 8; i += 2)
    {
        tab[i] = k_malloc(size - 1);
    }
    for (int i = 0; i < 8; i++)
    {
        k_free(tab[i]);
    }
    // print_size_mem();
}

void allocator_test_9()
{
    void *tab[100] = {0};
    uint32_t size = 50;
    for (int i = 0; i < 100; i++)
    {
        tab[i] = k_malloc(size);
        assert(tab[i] != NULL);
    }

    for (int i = 99; i >= 0; i--)
    {
        k_free(tab[i]);
    }
}

void allocator_test_10()
{
    void *tab[100] = {0};
    uint32_t size = 156;
    for (int i = 0; i < 8; i++)
    {
        tab[i] = k_malloc(size);
        assert(tab[i] != NULL);
    }
    for (int i = 0; i < 8; i += 2)
    {
        k_free(tab[i]);
    }
    for (int i = 0; i < 8; i++)
    {
        mem_slice *tmp = tab[i] - 14;
        if (i % 2 == 0)
            assert((tmp->size == size + 14) && (tmp->is_free == true));
        if (i % 2 == 1)
            assert((tmp->size == size + 14) && (tmp->is_free == false));
    }
    for (int i = 0; i < 8; i += 2)
    {
        tab[i] = k_malloc(27);
        
    }
    for (int i = 7; i >= 0; i--)
    {
        k_free(tab[i]);
        print_linked();
    }
    print_linked();
}

// need to write more tests to make sure there is no memort leak.


void allocator_test_11(){

    void* p = k_malloc((end-start)/2 - sizeof(mem_slice));
    assert(p!=NULL);
    void* p1 = k_malloc((end-start)/2 - sizeof(mem_slice));
    assert(p!=NULL);
    k_free(p);
    p = k_malloc((end-start)/2 - sizeof(mem_slice) - 15);
    assert(p!=NULL);
    k_free(p);
    k_free(p1);
}



void memory_allocator_tests()
{
    // mem_slice* tmp=memory_head;
    allocator_test_0();
    allocator_test_1();
    allocator_test_2();
    allocator_test_3();
    allocator_test_4();
    allocator_test_5();
    allocator_test_6();
    allocator_test_7();
    allocator_test_8();
    allocator_test_9();
    allocator_test_10();
    allocator_test_11();

}




// todo added the hash type shit and need to figure out how to update it.


//need to cover more testcases.
