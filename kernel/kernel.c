#include "common.h"
#include <stdarg.h>

#define VIDEO_MEMORY 0xb8000






#define ACCESS_BYTE_A               1<<0
#define ACCESS_BYTE_RW              1<<1
#define ACCESS_BYTE_DC              1<<2
#define ACCESS_BYTE_E               1<<3
#define ACCESS_BYTE_S               1<<4
#define ACCESS_BYTE_DPL_1           1<<5
#define ACCESS_BYTE_DPL_2           1<<6
#define ACCESS_BYTE_P               1<<7

#define FLAGS_RESERVED              1<<4
#define FLAGS_L                     1<<5
#define FLAGS_BD                    1<<6
#define FLAGS_G                     1<<7

#define KERNEL_CS                   0x8
#define KERNEL_DS                   0x10
#define USER_CS                     (0x18 | 3)
#define USER_DS                     (0x20 | 3)
#define MASTER_PIC_COMMAND_PORT     0x20
#define MASTER_PIC_DATA_PORT        0x21
#define SLAVE_PIC_COMMAND_PORT      0xA0
#define SLAVE_PIC_DATA_PORT         0xA1
#define PIC_EOI_COMMAND             0x20 //end of interrupt
#define PIC_INIT_COMMAND            0x11
#define ASCII_0                     '0'
#define ASCII_9                     (ASCII_0 + 9)
//a tipical irq handler
// void handler(){
//     do stuff...;
//     sendendofinterrupt()
// }


uint32_t tiks = 0;
void assert_correct_state(void);
void kill();
typedef struct gdt_entry{
    uint16_t limit_1;
    uint16_t base_1;
    uint8_t base_2;
    uint8_t access_bytes;
    uint8_t flags_limit;
    uint8_t base_3;
}__attribute__((packed)) gdt_entry;


typedef struct tss_struct{
    uint16_t link;
    uint16_t reserved;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved_0;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved_1;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved_2;
    uint32_t unused[19];
}__attribute__((packed))tss_struct;


typedef struct gdt_desciptor{
    uint16_t size;
    gdt_entry* ptr; 
}__attribute__((packed)) gdt_desciptor;


typedef struct idt_entry{
    uint16_t offset_1;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_2;
}__attribute__((packed)) idt_entry;

typedef struct idt_desciptor{
    uint16_t size;
    idt_entry* ptr; 
}__attribute__((packed)) idt_desciptor;

typedef struct registers{
    // TODO : not the same regs are pushed when there is PL change
    uint32_t edi,esi,ebp,reserved,ebx,edx,ecx,eax,interrupt_number,error_code,eip;
    uint8_t  cs,reserved1;
    uint32_t eflags;
}__attribute__((packed)) registers;

//---------------------------------------------
//      Gloabal variables    
//---------------------------------------------
uint32_t pos_x = 0;
uint32_t pos_y = 0;
gdt_entry gdt[6] __attribute__((aligned(8)))={0};
gdt_desciptor gdt_d;
idt_entry idt[256] __attribute__((aligned(8)))={0};
idt_desciptor idt_d;
tss_struct tss = {0};
uint8_t master_pic_mask=0;
uint8_t slave_pic_mask=0;
//---------------------------------------------
//      functions   
//---------------------------------------------

void load_gdt(gdt_desciptor* gdt_d);
void load_idt(idt_desciptor* idt_d);
void handler(registers* regs);
typedef void (*irq_handler)(registers* regs);
irq_handler irq_handler_table[16]={0};
void get_to_user_space(void);
void load_tss(void);
uint8_t inb(uint16_t port);
void outb(uint16_t port,uint8_t data);

void div_0(void);
void initialize_tss(){
    tss.ss0 = KERNEL_DS;
}


// for real hardware it may takes time to do io operations
void io_wait(void)
{
    outb(0x80, 0);
}


void scheduler(void);
/*===============================================================================================*/

void register_irq_handler(int irq_number,irq_handler handler){
    assert((irq_number >= 0) && (irq_number < 16));
    irq_handler_table[irq_number] = handler;
    return;
}
#define FREQ 50
#define DEFAULT_FREQ 0x1234DD //Hz (environ 1,19 MHz)
void change_freq(){
    outb(0x43,0x34); //command to change the frequence 0x43 is the command port
    uint16_t value = (uint16_t)(DEFAULT_FREQ/FREQ);
    outb(0x40,(uint8_t)value);
    outb(0x40,(uint8_t)(value>>8));
    return; 
}


void print_time(){
    uint32_t tmp_x = pos_x;
    uint32_t tmp_y = pos_y;
    pos_x = 70;
    pos_y = 0;
    uint32_t seconds = (tiks/50)%60;
    uint32_t minutes = (tiks/50)/60;
    k_print("%d :%d ",minutes,seconds);
    pos_x = tmp_x;
    pos_y = tmp_y;
}

void timer_handler(registers* regs){
    ++tiks;
    assert_correct_state();
    //k_print("hi");
    print_time();
    scheduler();
}

typedef struct link{
    struct link* next;
    struct link* prev;
}__attribute__((packed))link;


#define __init__link__(name) link name = {&name,&name};


#define __get__struct__(ptr_link,type,link_field) (type*)((void*)ptr_link  -  (void*)&((type*)0)->link_field)


#define __add__link__(head,ptr_elem,type,link_field,prio_field) \
    do \
    {  \
        link* tmp = (link*)head;\
        type* element = (type*)ptr_elem;\
        link* ptr_link = (link*)(&(element->link_field));\
        tmp = tmp->next;\
        assert(ptr_link->next == (void*)0 && ptr_link->prev == (void*)0);\
        while (tmp != head &&  ((__get__struct__(tmp,type,link_field))->prio_field < element->prio_field))\
        {\
            tmp=tmp->next;\
        }\
        ptr_link->next = tmp;\
        ptr_link->prev = tmp->prev;\
        tmp->prev->next = ptr_link;\
        tmp->prev = ptr_link;\
    } while (0);
    
static __inline__ int queue_empty(link *head)
{
	return (head->next == head);
}


/**
 * Retrait de l'�l�ment prioritaire de la file
 * head      : pointeur vers la t�te de liste
 * type      : type de l'�l�ment � retourner par r�f�rence
 * listfield : nom du champ du lien de chainage
 * retourne un pointeur de type 'type' vers l'�l�ment sortant
 */
#define queue_out(head, type, listfield) \
	(type *)__queue_out(head, (unsigned long)(&((type *)0)->listfield))

/**
 * Fonction � usage interne utilis�e par la macro ci-dessus
 * head : pointeur vers la t�te de liste
 * diff : diff�rence entre l'adresse d'un �l�ment et son champ de
 *        type 'link' (cf macro list_entry)
 */
static __inline__ void *__queue_out(link *head, unsigned long diff)
{
	// On r�cup�re un pointeur vers le maillon
	// du dernier �l�ment de la file.
	unsigned long ptr_link_ret = (unsigned long)(head->prev);

	// Si la file est vide, on retourne le pointeur NULL.
	if (queue_empty(head))
		return ((void *)0);

	// Sinon on retire l'�l�ment de la liste,
	head->prev = head->prev->prev;
	head->prev->next = head;

	((link *)ptr_link_ret)->prev = 0;
	((link *)ptr_link_ret)->next = 0;

	// Et on retourne un pointeur vers cet �l�ment.
	return ((void *)(ptr_link_ret - diff));
}



void write_char(char c){
    if(c == '\n'){
        pos_x = 0;
        pos_y++;
        return;
    }
    if(c == '\0') return;
    uint16_t* pos = (uint16_t*)(VIDEO_MEMORY + 2*(80*pos_y + pos_x));
    *(pos) = (uint16_t) c;
    *((uint8_t*)pos+1)=0b00001111; 
    if(pos_x+1 == 80){
        pos_x =0;
        pos_y = pos_y+1;
    }
    else pos_x++; 
}

void print_s(char* s){
    while(*s != '\0'){
        write_char(*(s++));
    }
}

void clear_screen(){
    for(uint8_t i = 0 ; i < 25 ; i++){
        for(uint8_t j = 0 ; j < 80 ; j++){
            uint16_t* pos = (uint16_t*)(VIDEO_MEMORY + 2*(80*i + j));
            uint16_t value = (uint16_t) ' ' | ((uint16_t)0b00001111 << 8);
            *(pos) = value; 
        }
    }
    pos_x = 0;
    pos_y = 0;
}



void set_idt_entry(uint8_t index,uint16_t selector,uint32_t offset){
    idt[index].flags = 0b10001110;
    idt[index].offset_1 = (uint16_t)offset;
    idt[index].offset_2 = (uint16_t)(offset >> 16);
    idt[index].reserved = 0;
    idt[index].segment_selector = selector;
}

void unmask_irq(uint8_t irq){
    uint16_t port;
    uint8_t mask;
    if(irq < 8){
        port = MASTER_PIC_DATA_PORT;
        master_pic_mask = master_pic_mask & (~(1<<irq));
        mask = master_pic_mask;
    }
    else if(irq < 16){
        irq-=8;
        port = SLAVE_PIC_COMMAND_PORT;
        slave_pic_mask = slave_pic_mask & (~(1<<irq));
        mask = slave_pic_mask;
    }
    outb(port,mask);
}

void mask_irq(uint8_t irq){
    uint16_t port;
    uint8_t mask;
    if(irq < 8){
        port = MASTER_PIC_DATA_PORT;
        master_pic_mask = master_pic_mask | (1<<irq);
        mask = master_pic_mask;
    }
    else if(irq < 16){
        irq-=8;
        port = SLAVE_PIC_COMMAND_PORT;
        slave_pic_mask = slave_pic_mask | (1<<irq);
        mask = slave_pic_mask;
    }
    outb(port,mask);
}

void initialize_pic(){
    //initialization:
    outb(MASTER_PIC_COMMAND_PORT,PIC_INIT_COMMAND);
    outb(SLAVE_PIC_COMMAND_PORT,PIC_INIT_COMMAND);

    //set off-set
    outb(MASTER_PIC_DATA_PORT, 0x20);
    outb(SLAVE_PIC_DATA_PORT, 0x28);

    //map 
    outb(MASTER_PIC_DATA_PORT, 0x4);
    outb(SLAVE_PIC_DATA_PORT, 0x2);

    //use 8086  mode //i dont know what is it exacly
    //there are two types of IRQ ... nanobyte
    outb(MASTER_PIC_DATA_PORT, 0x1);
    outb(SLAVE_PIC_DATA_PORT, 0x1);

    //mask all IRQ
    outb(MASTER_PIC_DATA_PORT, 0xff);
    outb(SLAVE_PIC_DATA_PORT, 0xff);
    master_pic_mask = 0xff;
    slave_pic_mask = 0xff;

    //unmask_irq(0);
}

void set_gdt(){
    //null segment
    gdt[0].limit_1=0;
    gdt[0].base_1=0;
    gdt[0].base_2=0;
    gdt[0].access_bytes=0;
    gdt[0].flags_limit=0;
    gdt[0].base_3=0;
    
    //kernel code segment
    gdt[1].limit_1= 0xffff;
    gdt[1].base_1=0;
    gdt[1].base_2=0;
    gdt[1].access_bytes= ACCESS_BYTE_P | ACCESS_BYTE_S | ACCESS_BYTE_E | ACCESS_BYTE_RW ;
    gdt[1].flags_limit=0b00001111 | (FLAGS_G | FLAGS_BD);
    gdt[1].base_3=0;

    //kernel data and code segments:
    gdt[2].limit_1= 0xffff;
    gdt[2].base_1=0;
    gdt[2].base_2=0;
    gdt[2].access_bytes= ACCESS_BYTE_P | ACCESS_BYTE_S | ACCESS_BYTE_RW ;
    gdt[2].flags_limit=0b00001111 | (FLAGS_G | FLAGS_BD);
    gdt[2].base_3=0;

    //user code segment
    gdt[3].limit_1= 0xffff;
    gdt[3].base_1=0;
    gdt[3].base_2=0;
    gdt[3].access_bytes= ACCESS_BYTE_P | ACCESS_BYTE_S | ACCESS_BYTE_E | ACCESS_BYTE_RW | ACCESS_BYTE_DPL_1 | ACCESS_BYTE_DPL_2;
    gdt[3].flags_limit=0b00001111 | (FLAGS_G | FLAGS_BD);
    gdt[3].base_3=0;

    //user data and code segments:
    gdt[4].limit_1= 0xffff;
    gdt[4].base_1=0;
    gdt[4].base_2=0;
    gdt[4].access_bytes= ACCESS_BYTE_P | ACCESS_BYTE_S | ACCESS_BYTE_RW | ACCESS_BYTE_DPL_1 | ACCESS_BYTE_DPL_2;
    gdt[4].flags_limit=0b00001111 | (FLAGS_G | FLAGS_BD);
    gdt[4].base_3=0;

    //TSS gate segment
    initialize_tss();
    uint32_t tss_size = sizeof(tss);
    gdt[5].limit_1= (uint16_t)tss_size;
    gdt[5].base_1=(uint16_t)((uint32_t)&tss);
    gdt[5].base_2=(uint8_t)(((uint32_t)&tss) >> 16);
    gdt[5].access_bytes= ACCESS_BYTE_P | 0x9;
    gdt[5].flags_limit= 0;
    gdt[5].base_3=(uint8_t)(((uint32_t)&tss) >> 24);

    //TODO : add the user entries, and TSS if needed in the user processes scheddulling
    //TODO : verify that the gdt is 8 byte-alaigned (the gnu attribute __atribute__((aligned(8)))
    gdt_d.size = sizeof(gdt_entry)*6;
    gdt_d.ptr = gdt; 
    load_gdt(&(gdt_d));
    load_tss();


    return;
}

void send_end_of_interrupt(uint8_t irq){
    if(irq >= 8)
        outb(SLAVE_PIC_COMMAND_PORT,PIC_EOI_COMMAND);
    outb(MASTER_PIC_COMMAND_PORT,PIC_EOI_COMMAND);
}

void set_idt(){
    set_idt_entries;
    idt_d.size = sizeof(idt_entry)*256;
    idt_d.ptr = idt; 
    load_idt(&(idt_d));
}
void handler(registers* reg){
    uint8_t interrupt_number = reg->interrupt_number;
    if(interrupt_number < 32){
        k_print("exceptions %x\n",interrupt_number);
        //while(1) halt();
    }
    else if(interrupt_number < 48){
        uint8_t irq = interrupt_number - 32;
        send_end_of_interrupt(irq);
        irq_handler_table[irq](reg);
        //we send the eof and we go to the handler im not sure if that will cause problem, for schudilling i think its necessary
    }
}

char* BASE10_NUMBERS = "0123456789";
char* BASE16_NUMBERS = "0123456789abcdef";
void print_atoi_10(int num){
    char num_tab[10]={0};
    int i=9;
    int r=0;
    do
    {
        r = num%10;
        num_tab[--i] = BASE10_NUMBERS[r];

    } while ((num=num/10) != 0);
    print_s((char*)(num_tab+i));
}
void print_atoi_16(int num){
    char num_tab[10]={0};
    int i=9;
    int r=0;
    do
    {
        r = num%16;
        num_tab[--i] = BASE16_NUMBERS[r];

    } while ((num=num/16) != 0);
    print_s((char*)(num_tab+i));
}


__init__link__(zombie_link)







void k_print(char* s,...){
    va_list params;
    va_start(params,s);
    while(*s != '\0'){
        if(*s == '%'){
            s++;
            switch (*s)
            {
            case 's':
            {
                s++;
                char* a = va_arg(params, char*);
                print_s(a);
                break;
            }  
            case 'd':
            {
                s++;
                int num = va_arg(params,int);
                print_atoi_10(num);
                break;
            } 
            case 'x':
            {
                s++;
                int num = va_arg(params,int);
                print_atoi_16(num);
                break;
            }          
            default:
                break;
            }
        }
        else write_char(*(s++));
    }
    va_end(params);
    return;
}


/*======================================================================================================*/
#define MAX_PROC_NUM 30
typedef enum state{
    actif,
    activable,
    endormi,
    zombie
}state;

typedef struct process{
    uint32_t reg[8];
    char name[8];
    int id;
    state etat;
    uint32_t prio;
    link a_link;
    uint32_t rev;
    struct process* parent;
    link* child_head;
    link  child_link;
    int retval;
}__attribute((packed)) process;



/*
 * eax
 * ebx
 * ecx
 * edx
 * esi
 * edi
 * ebp
 * esp  
 */




__init__link__(activable_link)
__init__link__(endormi_link)

process* p_actif = (void*)0;
process* p_prev = (void*)0;
void k_strcpy(char* dst,char* src){
    while(*src != '\0'){
        *dst++ = *src++;
    }
}

process* table_process[MAX_PROC_NUM] ={0};
/*
 * processes goes from 0 to MAX_PROC_NUM.
 */

uint32_t get_proc_id(){
    for(int i=0; i< MAX_PROC_NUM; i++){
        if(table_process[i] == (process*)0) return i;
    }
    return -1;
}



void add_activable_process(process* p){
    p->etat = activable;
    __add__link__((&activable_link),p,process,a_link,prio);
}


void pr(){
    link* tmp = activable_link.next;
    while (tmp != &activable_link)
    {
        process* proc = __get__struct__(tmp,process,a_link);
        k_print("%s->",proc->name);
        tmp= tmp->next;
    }
    k_print("\n");
}
void scheduler(){
    //if there is an endormi process that his time came, add it to activable

    for(;;){
        process* proc= queue_out((&endormi_link),process,a_link);
        if (proc == (void*)0) break;
        else if ((-1*(proc->rev)) > tiks) {
            __add__link__((&endormi_link),proc,process,a_link,rev);
            break;
        }
        add_activable_process(proc);
    }

    if(p_actif == (void*)0) return;
    p_prev = p_actif;
    if(p_prev->etat == actif) add_activable_process(p_prev);
    //pr();
    p_actif = queue_out((&activable_link),process,a_link);
    p_actif->etat =actif;
    ctx_sw();
}


void wait_clock(uint32_t clock){
    assert(clock>0);
    p_actif->etat = endormi;
    p_actif->rev = -1*(tiks+clock);
    __add__link__((&endormi_link),p_actif,process,a_link,rev);
    scheduler();
}

void return_address(){
    k_print("process_returned\n");
    kill();
}

void exit_(int retval){
    k_print("process exited\n");
    p_actif->etat = zombie;
    p_actif->retval = retval;
    __add__link__((&zombie_link),p_actif,process,a_link,prio);
    scheduler();
}

int start(void* function,uint32_t prio, char* name,void* arg){
    uint32_t id=0, stack_size = 512, esp=0, top_stack=0;
    if( (id = get_proc_id()) == -1) return id;
    process* new_proc = k_malloc(sizeof(process));
    k_strcpy(new_proc->name,name);
    new_proc->retval = 0;
    new_proc->id = id;
    new_proc->prio = prio;
    new_proc->rev = 0;
    uint32_t* stack = k_malloc(stack_size+12);//after the ret the 
    top_stack = (uint32_t)stack + stack_size +12 - 4;
    esp = top_stack-8;
    new_proc->reg[7]=esp;
    add_activable_process(new_proc);
    *(uint32_t*)(top_stack - 4)=(uint32_t)&exit_;
    *(uint32_t*)(top_stack)=(uint32_t)arg;
    *(uint32_t*)esp = (uint32_t)function;
    table_process[id] = new_proc;

    /*child stuff*/
    new_proc->parent = p_actif;

    __init__link__(head);
    new_proc->child_head = &head;
    __add__link__(p_actif->child_head,new_proc,process,child_link,prio);

    scheduler();
    return id;
}




int mon_pid(){
    return p_actif->id;
}
char* mon_nom(){
    return p_actif->name;
}

#define SEC_TO_TIKS 50

// void proc(void) {
//     int i =0;
//     for (;;) {
//         k_print("[%s]\n", p_actif->name);
//         wait_clock(SEC_TO_TIKS*2);
//         if(i == 2){
//             kill();
//         }
//         i++;
//     }
// }

int waitpid(int pid, int *retvalp){
    link* tmp = p_actif->child_head->next;
    process* tmp_proc = (void*)0;
    process* z_proc = (void*)0;
    if(pid<0){
        if(queue_empty((p_actif->child_head))) return -1;
        while(1){
            tmp = p_actif->child_head->next;
            while(tmp != (p_actif->child_head)){
                tmp_proc = __get__struct__(tmp,process,child_link);
                if(tmp_proc->etat == zombie) {
                    z_proc = tmp_proc;
                    break; /*need to destroy the zombie*/
                }
                tmp=tmp->next;
            }
            if(z_proc != (void*)0) break;
            scheduler();        
        }
    }
    else if(pid>0){
        if(table_process[pid]->parent != p_actif) return -1;
        while(1){
            if(table_process[pid]->etat == zombie) {
                z_proc = table_process[pid];
                break;
            }
            else scheduler();
        }  
    }
    assert(z_proc != (void*)0);
    *retvalp = z_proc->retval;
    return z_proc->id;
}


void hii(){
    k_print("[%s] ill return bye, state:%d \n", p_actif->name,p_actif->id);
}

void proc2(uint32_t a) {
    wait_clock(5*SEC_TO_TIKS);
    k_print("[%s] receiced value : %d \n",p_actif->name,a);
    wait_clock(5*SEC_TO_TIKS);
    exit_(4);
}

void proc(void) {
    //int i = 0;
    start(proc2,0,"proc2",(void*)10);
    int a = 0;
    int pid=0;
    pid = waitpid(-1,&a);
    k_print("[%s]pid:%d,retval:%d \n",p_actif->name,pid,a);

}




void idle(void)
{
    start(proc,0,"proc_1",0);
    // link* tmp = p_actif->child_head->next;
    // process* tmp_proc = (void*)0;
    while(1){
        // while(tmp != p_actif->child_head){
        //     tmp_proc = __get__struct__(tmp,process,child_link);
        //     k_print("[%s] i have child %s\n",p_actif->name,tmp_proc->name);
        //     tmp=tmp->next;
        // }
        // tmp = p_actif->child_head->next;
        sti();
        hlt();
        cli();
    }
}




void __init__proc(void){
    uint32_t id=0, stack_size = 512, esp=0;
    if( (id = get_proc_id()) == -1) return;
    assert(id == 0);
    process* new_proc = k_malloc(sizeof(process));
    k_strcpy(new_proc->name,"idle");
    new_proc->id = id;
    new_proc->prio = 0;

    uint32_t* stack = k_malloc(stack_size);
    esp = (uint32_t)stack + stack_size -1;
    new_proc->reg[7]=esp;
    new_proc->etat = actif;
    *(uint32_t*)esp = (uint32_t)idle;
    table_process[id] = new_proc;
    __init__link__(head);
    new_proc->child_head = &head;
    p_actif = new_proc;
    
    idle();
}


/*
 * i have p_actif and p_next and table_process need to be updated.
 */


void initialize_irq(){
    register_irq_handler(0,timer_handler);
    unmask_irq(0);
    change_freq();
    
}

void assert_correct_state(){
    link* l = activable_link.next;
    while(l != &activable_link){
        assert(((__get__struct__(l,process,a_link))->etat) == activable);
        l=l->next;
    }
    l = endormi_link.next;
    while(l != &endormi_link){
        assert(((__get__struct__(l,process,a_link))->etat) == endormi);
        l=l->next;
    }
    l = zombie_link.next;
    while(l != &zombie_link){
        assert(((__get__struct__(l,process,a_link))->etat) == zombie);
        l=l->next;
    }
}

void kill(){
    p_actif->etat = zombie;
    __add__link__((&zombie_link),p_actif,process,a_link,prio);
    scheduler();
}
/*========================================================================================================*/
uint32_t j = 0;
void kernel_main(){
    clear_screen();
    set_gdt();
    // TODO : initialize the PIC device
    set_idt();   
    initialize_pic();
    initialize_irq();
    //k_print("%d ",4);
    //sti();
    //get_to_user_space();
    //div_0();
    __init__();
    __init__proc();
    //k_print("there is an error\n");
    while(1);
    while(1){
        hlt(); 
    }
}



//note : the DPL in the kernel segments are set to 0, so only the kernel can execute code in those segments.
// get to user mode and and generate an execption, go to kernel mode, then go back to user mode.


// TODO : i need to think to update the esp0 in the stack after the interrupt ends i think that would be in common_isr.
// ideas:
//      what we have now: kernel esp is stored in the TSS, (Q : when should I update it)
//      create process schedulling:
//      when a user process get interrupted, it get the kernel esp from the tss, it go to the handler. kernel cs get updated.
//      the kernel execute the handler.
//      we get back to iret, then thte value in the stack are:
//      old eip,old cs, old eflags, old esp, old ss
//      we excute the iret and we go back to where we were in the process interrupted.
//      warnings:
//      but we need to make sur that all the processes kept the same value as before the.
//      also not the same stuffs get pushed depending on if the interrupt need a change on the previledge level or not.
//      assert that the cs are correct when after the interrupt occurs and after returning from it (after iret)
//      
//      when there is a level change my current regs structure does not store ss, and esp
//
//      for better testing i need to implement a better print_s.
//      i should work with timer interrupts.(so i need to initialize the PIC device).


//scheddyler ideas:
//every process has a way to store kernel stack pointer in its structure.
//when a process get interupted its get it goes to kernel mode with the esp from the tss.
//then it got to scheduller where:
//current regs are stored in the process struct. the kernel esp is kept inside the process struct too.
//the new process get the regs from its struct and set the kernel esp too(i think from its struct).
//then do the ret and jump back to after the scheduller.
//do get to the interrupt handler where it should send end of interrupt and do the iret.



/*
 * todo need to verify that the context switch keep the register unchanged.
 */


/*
 * write a tool to show the memory state.s
 */




//passing arguments to processes.
//exit and return.
