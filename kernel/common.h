#ifndef __COMMON__H
#define __COMMON__H

#define DECL_ISR(i) void isr_##i(void);

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






#endif
