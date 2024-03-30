.set ALIGN,    1<<0             
.set MEMINFO,  1<<1      
.set FLAGS,    ALIGN | MEMINFO 
.set MAGIC,    0x1BADB002      
.set CHECKSUM, -(MAGIC + FLAGS) 

//kernel segments
.set KERNEL_CS, 0x8
.set KERNEL_DS, 0x10
.set USER_CS,   0x18 | 3
.set USER_DS,   0x20 | 3

.section .multiboot
    .align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM


.extern p_actif
.extern p_next

.section .bss
    .align 16
    stack_bottom:
    .skip 16384 # 16 KiB
    stack_top:
    .align 16
    user_stack_bottom:
    .skip 16384 # 16 KiB
    user_stack_top:

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

.global sti
sti:
    sti
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


no_error_isr 32
no_error_isr 33
no_error_isr 34
no_error_isr 35
no_error_isr 36
no_error_isr 37
no_error_isr 38
no_error_isr 39
no_error_isr 40
no_error_isr 41
no_error_isr 42
no_error_isr 43
no_error_isr 44
no_error_isr 45
no_error_isr 46
no_error_isr 47



.global common_isr
common_isr:
    pusha
    push %esp
    call handler
    //to remove the esp pushed(pointer the structure)
    add $4,%esp
    popa
    //remove error code, and the interrupt number.
    add $8,%esp
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


.global get_to_user_space
get_to_user_space:
    //save the kernel stack pointer to the tss.
    movl $tss,%eax
    movl %esp,4(%eax)

    mov $USER_DS,%eax
    mov  %eax,%ds
    mov  %eax,%es
    mov  %eax,%gs
    push %eax
    push $user_stack_top
    pushf
    push $USER_CS
    push $user_function
    iret



//tss is the variable name in c and is can be used a pointer to the structer in assembly
//new stuff!!
.global load_tss
load_tss:
    push %eax
    mov $0x28,%eax
    ltr %ax
    pop %eax
    ret

.global hi
hi:
    jmp hi

.global user_function
user_function:
    call hi
    ret


// outb(uint16_t port, uint8_t data);
.global outb
outb:
    //eax caller saved
    push %ebp
    mov %esp,%ebp

    xor %eax,%eax
    xor %edx,%edx
    movb 12(%ebp),%al
    movw 8(%ebp),%dx
    outb %al,%dx

    mov %ebp,%esp
    pop %ebp
    ret


//inb(uint16_t port)
.global inb
inb:
    //eax caller saved
    push %ebp
    mov %esp,%ebp

    xor %eax,%eax
    xor %edx,%edx
    movw 8(%ebp),%dx
    inb  %dx,%al

    mov %ebp,%esp
    pop %ebp
    ret



.global ctx_sw
ctx_sw:
    pushl %eax
    movl $p_prev,%eax
    movl (%eax),%eax
    popl (%eax)
    movl %ebx,4(%eax)
    movl %ecx,8(%eax)
    movl %edx,12(%eax)
    movl %esi,16(%eax)
    movl %edi,20(%eax)
    movl %ebp,24(%eax)
    movl %esp,28(%eax)

    movl $p_actif,%eax
    movl (%eax),%eax
    movl 28(%eax),%esp
    movl 24(%eax),%ebp
    movl 20(%eax),%edi
    movl 16(%eax),%esi
    movl 12(%eax),%edx
    movl 8(%eax),%ecx
    movl 4(%eax),%ebx
    movl 0(%eax),%eax
    ret
    



/*
 * regs 
 * eax
 * ebx
 * ecx
 * edx
 * esi
 * edi
 * ebp
 * esp  
*/
