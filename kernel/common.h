#ifndef __COMMON__H
#define __COMMON__H

#include <stdarg.h>

#define DECL_ISR(i)  void isr_##i(void);

                    
typedef unsigned short uint16_t;
typedef unsigned       uint32_t;
typedef unsigned char  uint8_t;


void clear_screen();
void print_s(char* s);
void __init__();
void allocate();
void memory_allocator_tests();
void k_print(char* s,...);
#define assert(expr) \
                    do\
                    {\
                        if(!(expr)) print_s("error, assert condition not fullfilled\n");\
                    } while (0)


DECL_ISR(0);
DECL_ISR(1);
DECL_ISR(2);
DECL_ISR(3);
DECL_ISR(4);
DECL_ISR(5);
DECL_ISR(6);
DECL_ISR(7);
DECL_ISR(8);
DECL_ISR(9);
DECL_ISR(10);
DECL_ISR(11);
DECL_ISR(12);
DECL_ISR(13);
DECL_ISR(14);
DECL_ISR(15);
DECL_ISR(16);
DECL_ISR(17);
DECL_ISR(18);
DECL_ISR(19);
DECL_ISR(20);
DECL_ISR(21);
DECL_ISR(22);
DECL_ISR(23);
DECL_ISR(24);
DECL_ISR(25);
DECL_ISR(26);
DECL_ISR(27);
DECL_ISR(28);
DECL_ISR(29);
DECL_ISR(30);
DECL_ISR(31);

DECL_ISR(32);
DECL_ISR(33);
DECL_ISR(34);
DECL_ISR(35);
DECL_ISR(36);
DECL_ISR(37);
DECL_ISR(38);
DECL_ISR(39);
DECL_ISR(40);
DECL_ISR(41);
DECL_ISR(42);
DECL_ISR(43);
DECL_ISR(44);
DECL_ISR(45);
DECL_ISR(46);
DECL_ISR(47);



#define set_idt_entries \
                        set_idt_entry(0,KERNEL_CS,(uint32_t)isr_0);\
                        set_idt_entry(1,KERNEL_CS,(uint32_t)isr_1);\
                        set_idt_entry(2,KERNEL_CS,(uint32_t)isr_2);\
                        set_idt_entry(3,KERNEL_CS,(uint32_t)isr_3);\
                        set_idt_entry(4,KERNEL_CS,(uint32_t)isr_4);\
                        set_idt_entry(5,KERNEL_CS,(uint32_t)isr_5);\
                        set_idt_entry(6,KERNEL_CS,(uint32_t)isr_6);\
                        set_idt_entry(7,KERNEL_CS,(uint32_t)isr_7);\
                        set_idt_entry(8,KERNEL_CS,(uint32_t)isr_8);\
                        set_idt_entry(9,KERNEL_CS,(uint32_t)isr_9);\
                        set_idt_entry(10,KERNEL_CS,(uint32_t)isr_10);\
                        set_idt_entry(12,KERNEL_CS,(uint32_t)isr_12);\
                        set_idt_entry(13,KERNEL_CS,(uint32_t)isr_13);\
                        set_idt_entry(14,KERNEL_CS,(uint32_t)isr_14);\
                        set_idt_entry(15,KERNEL_CS,(uint32_t)isr_15);\
                        set_idt_entry(16,KERNEL_CS,(uint32_t)isr_16);\
                        set_idt_entry(17,KERNEL_CS,(uint32_t)isr_17);\
                        set_idt_entry(18,KERNEL_CS,(uint32_t)isr_18);\
                        set_idt_entry(19,KERNEL_CS,(uint32_t)isr_19);\
                        set_idt_entry(20,KERNEL_CS,(uint32_t)isr_20);\
                        set_idt_entry(21,KERNEL_CS,(uint32_t)isr_21);\
                        set_idt_entry(22,KERNEL_CS,(uint32_t)isr_22);\
                        set_idt_entry(23,KERNEL_CS,(uint32_t)isr_23);\
                        set_idt_entry(24,KERNEL_CS,(uint32_t)isr_24);\
                        set_idt_entry(25,KERNEL_CS,(uint32_t)isr_25);\
                        set_idt_entry(26,KERNEL_CS,(uint32_t)isr_26);\
                        set_idt_entry(27,KERNEL_CS,(uint32_t)isr_27);\
                        set_idt_entry(28,KERNEL_CS,(uint32_t)isr_28);\
                        set_idt_entry(29,KERNEL_CS,(uint32_t)isr_29);\
                        set_idt_entry(30,KERNEL_CS,(uint32_t)isr_30);\
                        set_idt_entry(31,KERNEL_CS,(uint32_t)isr_31);\
                        set_idt_entry(32,KERNEL_CS,(uint32_t)isr_32);\
                        set_idt_entry(33,KERNEL_CS,(uint32_t)isr_33);\
                        set_idt_entry(34,KERNEL_CS,(uint32_t)isr_34);\
                        set_idt_entry(35,KERNEL_CS,(uint32_t)isr_35);\
                        set_idt_entry(36,KERNEL_CS,(uint32_t)isr_36);\
                        set_idt_entry(37,KERNEL_CS,(uint32_t)isr_37);\
                        set_idt_entry(38,KERNEL_CS,(uint32_t)isr_38);\
                        set_idt_entry(39,KERNEL_CS,(uint32_t)isr_39);\
                        set_idt_entry(40,KERNEL_CS,(uint32_t)isr_40);\
                        set_idt_entry(41,KERNEL_CS,(uint32_t)isr_41);\
                        set_idt_entry(42,KERNEL_CS,(uint32_t)isr_42);\
                        set_idt_entry(43,KERNEL_CS,(uint32_t)isr_43);\
                        set_idt_entry(44,KERNEL_CS,(uint32_t)isr_44);\
                        set_idt_entry(45,KERNEL_CS,(uint32_t)isr_45);\
                        set_idt_entry(46,KERNEL_CS,(uint32_t)isr_46);\
                        set_idt_entry(47,KERNEL_CS,(uint32_t)isr_47);\




#endif
