#ifndef __link__
#define __link__

typedef struct link{
    struct link* prev;
    struct link* next;
}__attribute__((packed))link;


#define __init__(name)  link name = {&name,&name};

#define queue_add(ptr_elem, head, type, listfield, priofield) \
        do{ \
            
        }while(0);
                        


#endif