/*
 * link.c : library help linking data structure in order.
 */




typedef struct link{
    struct link* next;
    struct link* prev;
}__attribute__((packed))link;



#define __init__link__(name) link name = {&name,&name}; /*name is the head*/



/*
 * add_element: add an element to linked list given.
 * params : ptr_elem,head,type,link_field,prio_field
 * ptr element pointer to the element we want to chain.
 * head of type link* the head.
 * type type* is the type of ptr_elem
 * the type element should have a link field not a link *field
*/


#define add_element(ptr_ele,head,type,link_field,prio_field) \
    do \
    {  \
        link* tmp = (link*) head;\
        type* element = (type*) ptr_elem;\
        link  element_link = (link) (ptr_elem->link_field);\
        assert(element_link.next == NULL && element_link.prev == NULL);\
        while(){
            tmp=tmp->next;
        }

    } while (0);


#define get_elem(typ,field) \
    do\
    {\
       
    } while (0);
    