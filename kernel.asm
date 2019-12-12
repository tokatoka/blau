bits 32
section .text
        ;multiboot spec
        align 4
        dd 0x1BADB002              ;magic
        dd 0x00                    ;flags
        dd - (0x1BADB002 + 0x00)   ;checksum. m+f+c should be zero

global start
global keyboard_handler
global timer_handler
global read_port
global write_port
global load_idt
global read_gdt

extern kmain 		;this is defined in the c file
extern keyboard_handler_main
extern timer_handler_main

read_port:
	mov edx, [esp + 4]
			;al is the lower 8 bits of eax
	in al, dx	;dx is the lower 16 bits of edx
	ret

write_port:
	mov   edx, [esp + 4]
	mov   al, [esp + 4 + 4]
	out   dx, al
	ret

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti 				;turn on interrupts
	ret

read_gdt:
	mov edx, [esp + 4]
	sgdt [edx]
	ret

keyboard_handler:
	pusha
	call    keyboard_handler_main
	popa
	iretd

timer_handler:
	pusha
	call    timer_handler_main
	popa
	iretd

start:
	cli 				;block interrupts
	lgdt [gdt_descr]
	mov esp, stack_space
	call kmain
	hlt 				;halt the CPU

gdt_descr:
	dw	16*8-1
	dd	gdt
	dw 	0

gdt:
	dq	0x0000000000000000
	dq	0x00cf9a000000ffff
	dq	0x00cf92000000ffff
	dq	0x00cffa000000ffff
	dq	0x00cff2000000ffff
	dq  0
	dq  0
	dq  0
	dq  0
	dq  0
	dq  0
	dq  0
	dq  0
	dq  0
	dq  0
	dq  0


section .bss
resb 8192; 8KB for stack
stack_space: