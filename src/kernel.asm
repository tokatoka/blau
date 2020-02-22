%define MULTIBOOT_PAGE_ALIGN 0x1
%define MULTIBOOT_MEMORY_INFO 0x2

%define MULTIBOOT_HEADER_FLAGS (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO)

bits 32
section .text
        ;multiboot spec
        align 4
        dd 0x1BADB002
        dd 0x3
        dd -(0x1BADB002 + 0x3)

global start
global keyboard_handler
global timer_handler
global read_port
global write_port
global load_idt
global read_gdt
global DIVIDE_handler
global DEBUG_handler
global NMI_handler
global BPKPT_handler
global OVLOW_handler
global BOUND_handler
global ILLOP_handler
global DEVICE_handler
global DBLFLT_handler
global TSS_handler
global SEGNP_handler
global STACK_handler
global GPFLT_handler
global PGFLT_handler
global FPERR_handler
global MCHK_handler
global SIMDERR_handler
global ALIGN_handler
global enable_paging
global haltloop
global read_eflags
global load_master_pde
global do_jump_user_function
global syscall_handler

extern kmain 		;this is defined in the c file
extern keyboard_handler_main
extern timer_handler_main
extern kernel_panic
extern DBLFLT_handler_main
extern PGFLT_handler_main
extern panic
extern trap_handler_main

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

read_eflags:
	pushf
	pop eax
	ret

read_gdt:
	mov edx, [esp + 4]
	sgdt [edx]
	ret

DIVIDE_handler:
	pusha
	call panic
	popa
	iret

DEBUG_handler:
	jmp DEBUG_handler
	iret

NMI_handler:
	jmp NMI_handler
	iret

BPKPT_handler:
	push 0
	push 3
	jmp push_tf_and_jump

OVLOW_handler:
	jmp OVLOW_handler
	iret

BOUND_handler:
	jmp BOUND_handler
	iret

ILLOP_handler:
	jmp ILLOP_handler
	iret

DEVICE_handler:
	jmp DEBUG_handler
	iret

DBLFLT_handler:
	pusha
	call DBLFLT_handler_main
	popa
	iret

TSS_handler:
	jmp TSS_handler
	iret

SEGNP_handler:
	jmp SEGNP_handler
	iret

STACK_handler:
	jmp STACK_handler
	iret

GPFLT_handler:
	jmp GPFLT_handler
	iret

PGFLT_handler:
	push 0
	push 13
	jmp push_tf_and_jump

FPERR_handler:
	jmp FPERR_handler
	iret

ALIGN_handler:
	jmp ALIGN_handler
	iret

MCHK_handler:
	jmp MCHK_handler
	iret

SIMDERR_handler:
	jmp SIMDERR_handler
	iret

syscall_handler:
	push 0
	push 0x80
	jmp push_tf_and_jump


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

push_tf_and_jump:
	push ds
	push es
	pusha
	push 0x10
	pop ds
	push 0x10
	pop es
	push esp
	call trap_handler_main
	popa
	pop es
	pop ds
	jmp haltloop

do_jump_user_function:
	push 0
	push 0
	push 0
	push 0
	push 0
	push 0
	push 0
	push 0
	popad
	mov ax,0x23
	mov ds,ax
	mov es,ax
	mov edx, [esp + 4]
	push 0x23
	push eax
	push edx
	mov edx, [esp + 20] ;(esp + 8) + (12)
	pushf
	push 0x1b
	push edx
	xor edx, edx
	iretd


enable_paging:
	mov eax,0x1ff000
	mov cr3,eax
	mov eax,cr0
	or eax,0x80050020 ;enable paging, numeric error,alignment mask,write protect
	mov cr0,eax
	pop eax
	push eax
	add esp, 0xc0000000
	add eax, 0xc0000000
	jmp eax
	ret

haltloop:
	jmp haltloop

start:
	cli 				;block interrupts
	lgdt [gdt_descr]
	mov esp, 0x180000
	push ebx
	push eax
	call kmain
	hlt 				;halt the CPU

gdt_descr:
	dw	16*8-1
	dd	gdt
	dw 	0

gdt:
	dq	0x0000000000000000 ;NULL
	dq	0x00cf9a000000ffff ;kernel code
	dq	0x00cf92000000ffff ;kernel stack
	dq	0x00cffa000000ffff ;user code
	dq	0x00cff2000000ffff ;user stack
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
