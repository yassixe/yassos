.set ALIGN,    1<<0             
.set MEMINFO,  1<<1      
.set FLAGS,    ALIGN | MEMINFO 
.set MAGIC,    0x1BADB002      
.set CHECKSUM, -(MAGIC + FLAGS) 

//kernel segments
.set KERNEL_CS, 0x8
.set KERNEL_DS, 0x10


.section .multiboot
    .align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM


.section .bss
    .align 16
    stack_bottom:
    .skip 16384 # 16 KiB
    stack_top:

//setup the stack.
.section .text
.global _start
.type _start, @function
_start:
    cli
label:

    mov $stack_top, %esp
    call kernel_main

	cli
1:	hlt
	jmp 1b

.global halt
halt:
    hlt
    ret

.global load_gdt
load_gdt:
    push %ebp
    mov  %esp,%ebp

    push %eax
    mov  8(%ebp),%eax
    lgdt (%eax)
reload:
    mov $KERNEL_DS,%eax
    mov %eax,%ds
    mov %eax,%es
    mov %eax,%gs
    mov %eax,%ss
    ljmp $KERNEL_CS, $label_
label_:
    pop %eax
    mov %ebp,%esp
    pop %ebp
    ret


.global load_idt
load_idt:
    push %ebp
    mov  %esp,%ebp
    push %eax

    mov  8(%ebp),%eax
    lidt (%eax)

    pop %eax
    mov %ebp,%esp
    pop %ebp
    ret


.macro no_error_isr int_number
    .global isr_\int_number
    isr_\int_number:
        push $0
        push $\int_number
        jmp common_isr
.endm

.macro with_error_isr int_number
    .global isr_\int_number
    isr_\int_number:
        push $\int_number
        jmp common_isr
.endm


no_error_isr 0
no_error_isr 1
no_error_isr 2
no_error_isr 3
no_error_isr 4
no_error_isr 5
no_error_isr 6
no_error_isr 7
with_error_isr 8
no_error_isr 9
with_error_isr 10
with_error_isr 11
with_error_isr 12
with_error_isr 13
with_error_isr 14
no_error_isr 15
no_error_isr 16
with_error_isr 17
no_error_isr 18
no_error_isr 19
no_error_isr 20
with_error_isr 21
no_error_isr 22
no_error_isr 23
no_error_isr 24
no_error_isr 25
no_error_isr 26
no_error_isr 27
no_error_isr 28
with_error_isr 29
with_error_isr 30
no_error_isr 31

.global common_isr
common_isr:
    pusha
    push %esp
    call handler
    pop %esp
    popa
    iret

// eax,edx,ecx are caller seved registers in c_decl convention 
// so i dont need to worry about changing the here
.global div_0
div_0:
    push %ebp
    mov %esp,%ebp
    mov $4,%eax
    mov $0,%edx
    mov $0,%ecx
    div %ecx
    mov %ebp,%esp
    pop %ebp
    ret
    