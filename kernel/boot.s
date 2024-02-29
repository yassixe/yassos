.set ALIGN,    1<<0             
.set MEMINFO,  1<<1      
.set FLAGS,    ALIGN | MEMINFO 
.set MAGIC,    0x1BADB002      
.set CHECKSUM, -(MAGIC + FLAGS) 


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
    mov $8,%eax
    mov %eax,%ds
    mov %eax,%es
    mov %eax,%gs
    mov %eax,%ss
    ljmp $16, $label_
label_:
    pop %eax
    mov %ebp,%esp
    pop %ebp
    ret
