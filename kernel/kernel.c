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

typedef struct gdt_entry{
    uint16_t limit_1;
    uint16_t base_1;
    uint8_t base_2;
    uint8_t access_bytes;
    uint8_t flags_limit;
    uint8_t base_3;
}__attribute__((packed)) gdt_entry;


typedef struct gdt_desciptor{
    uint16_t size;
    gdt_entry* ptr; 
}__attribute__((packed)) gdt_desciptor;


typedef struct idt_entry{
    uint16_t offset_1;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint8_t offset_2;
}__attribute__((packed)) idt_entry;



//---------------------------------------------
//      Gloabal variables    
//---------------------------------------------
uint32_t pos_x = 0;
uint32_t pos_y = 0;
gdt_entry gdt[5] __attribute__((aligned(8)))={0};
gdt_desciptor gdt_d;

//---------------------------------------------
//      functions   
//---------------------------------------------
void halt(void);
void load_gdt(gdt_desciptor* gdt_d);

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
    
    //kernel data and code segments:
    //kernel code segment
    gdt[1].limit_1= 0xffff;
    gdt[1].base_1=0;
    gdt[1].base_2=0;
    gdt[1].access_bytes= ACCESS_BYTE_P | ACCESS_BYTE_S | ACCESS_BYTE_RW ;
    gdt[1].flags_limit=0b00001111 | (FLAGS_G | FLAGS_BD);
    gdt[1].base_3=0;

    //kernel data and code segments:
    //kernel code segment
    gdt[2].limit_1= 0xffff;
    gdt[2].base_1=0;
    gdt[2].base_2=0;
    gdt[2].access_bytes= ACCESS_BYTE_P | ACCESS_BYTE_S | ACCESS_BYTE_E | ACCESS_BYTE_RW ;
    gdt[2].flags_limit=0b00001111 | (FLAGS_G | FLAGS_BD);
    gdt[2].base_3=0;

    //TODO : add the user entries, and TSS if needed in the user processes scheddulling
    //TODO : verify that the gdt is 8 byte-alaigned (the gnu attribute __atribute__((aligned(8)))
    gdt_d.size = sizeof(gdt_entry)*5;
    gdt_d.ptr = gdt; 
    load_gdt(&(gdt_d));
    return;
}

void set_idt(){

}


void kernel_main(){
    clear_screen();
    print_s("hello from kernel!\n");
    print_s("hi i am working");
    set_gdt();
    print_s("hi i am working");
    while(1){
        halt();
    }
}