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
void halt(void);
void sti(void);
void load_gdt(gdt_desciptor* gdt_d);
void load_idt(idt_desciptor* idt_d);
void handler(registers* regs);
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

    unmask_irq(0);

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
        print_s("exceptions\n");
        //while(1) halt();
    }
    else if(interrupt_number < 48){
        uint8_t irq = interrupt_number - 32;
        (void)irq;
        print_s("irq\n");
        send_end_of_interrupt(irq);
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
            } 
            case 'x':
            {
                s++;
                int num = va_arg(params,int);
                print_atoi_16(num);
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
    activable
}state;

typedef struct process{
    uint32_t reg[8];
    char name[8];
    uint32_t id;
    state etat;
    uint32_t prio;
    
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

process* p_actif = (void*)0;
process* p_next = (void*)0;
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

int start(void(*function)(void),uint32_t prio, char* name){
    uint32_t id=0, stack_size = 512, esp=0;
    if( (id = get_proc_id()) == -1) return id;
    process* new_proc = k_malloc(sizeof(process));
    k_strcpy(new_proc->name,name);
    new_proc->id = id;
    new_proc->prio = prio;

    uint32_t* stack = k_malloc(stack_size);
    esp = (uint32_t)stack + stack_size -1;
    new_proc->reg[7]=esp;
    new_proc->etat = activable;
    *(uint32_t*)esp = (uint32_t)function;
    table_process[id] = new_proc;
    return id;
}

void proc(void){
    k_print("[%s]hi, its me\n",p_actif->name);
    p_next = table_process[0];
    p_actif = table_process[1];
    ctx_sw();
}

void idle(void){
    start(proc,0,"proc_1");
    start(proc,0,"proc_2");
    int a=3;
    p_next = table_process[1];
    ctx_sw();
    k_print("[%s]i got the hand back\n",p_actif->name);
    a++;
    k_print("a=%d \n",a);
    while(1);
}



void __init__proc(void){
    uint32_t id = start(idle,1,"idle");
    assert(id == 0);
    table_process[id]->etat = actif;
    p_actif = table_process[id];
}

/*========================================================================================================*/
uint32_t j = 0;
void kernel_main(){
    clear_screen();
    set_gdt();
    // TODO : initialize the PIC device
    set_idt();   
    initialize_pic();

    //sti();
    //get_to_user_space();
    //div_0();
    __init__();
    __init__proc();
    idle();

    while(1){
        halt(); 
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
