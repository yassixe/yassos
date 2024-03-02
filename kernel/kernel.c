#include "common.h"


#define VIDEO_MEMORY 0xb8000




typedef unsigned short uint16_t;
typedef unsigned       uint32_t;
typedef unsigned char  uint8_t;

#define ACCESS_BYTE_A       1<<0
#define ACCESS_BYTE_RW      1<<1
#define ACCESS_BYTE_DC      1<<2
#define ACCESS_BYTE_E       1<<3
#define ACCESS_BYTE_S       1<<4
#define ACCESS_BYTE_DPL_1   1<<5
#define ACCESS_BYTE_DPL_2   1<<6
#define ACCESS_BYTE_P       1<<7

#define FLAGS_RESERVED      1<<4
#define FLAGS_L             1<<5
#define FLAGS_BD            1<<6
#define FLAGS_G             1<<7

#define KERNEL_CS           0x8
#define KERNEL_DS           0x10
#define USER_CS             (0x18 | 3)
#define USER_DS             (0x20 | 3)

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
void div_0(void);

//---------------------------------------------
//      functions   
//---------------------------------------------
void halt(void);
void load_gdt(gdt_desciptor* gdt_d);
void load_idt(idt_desciptor* idt_d);
void handler(registers* regs);
void get_to_user_space(void);
void load_tss(void);

void initialize_tss(){
    tss.ss0 = KERNEL_DS;
}


void write_char(char c){
    if(c == '\n'){
        pos_x = 0;
        pos_y++;
        return;
    }
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

void initialize_pic(){

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

void set_idt(){
    set_idt_entries;
    idt_d.size = sizeof(idt_entry)*256;
    idt_d.ptr = idt; 
    load_idt(&(idt_d));
}
void handler(registers* reg){
    if(reg->interrupt_number < 32){
        print_s("exceptions\n");
        //while(1) halt();
    }
    else if(reg->interrupt_number < 48){
        print_s("irq\n");
    }
}

void kernel_main(){
    clear_screen();
    print_s("hello from kernel!\n");
    print_s("hi i am working\n");
    set_gdt();
    // TODO : initialize the PIC device
    set_idt();
    get_to_user_space();
    //div_0();
    while(1){
        halt();
    }
}



//note : the DPL in the kernel segments are set to 0, so only the kernel can execute code in those segments.
// get to user mode and and generate an execption, go to kernel mode, then go back to user mode.