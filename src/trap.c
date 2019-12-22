#include "task.h"
#include "io.h"
#include "syscall.h"
#include "cpu.h"
void print_tf(struct trapframe *tf){
	kprintf("tf -> edi : %x\n", tf -> edi);
	kprintf("tf -> esi : %x\n", tf -> esi);
	kprintf("tf -> ebp : %x\n", tf -> ebp);
	kprintf("tf -> oesp : %x\n", tf -> oesp);
	kprintf("tf -> ebx : %x\n", tf -> ebx);
	kprintf("tf -> ecx : %x\n", tf -> ecx);
	kprintf("tf -> eax : %x\n", tf -> eax);
	kprintf("tf -> es : %x\n", tf -> es);
	kprintf("tf -> ds : %x\n", tf -> ds);
	kprintf("tf -> trapno : %x\n", tf -> trapno);
	kprintf("tf -> errcode : %x\n", tf -> errcode);
	kprintf("tf -> eip : %x\n", tf -> eip);
	kprintf("tf -> cs : %x\n", tf -> cs);
	kprintf("tf -> eflags : %x\n", tf -> eflags);
	kprintf("tf -> esp : %x\n", tf -> esp);
	kprintf("tf -> ss : %x\n", tf -> ss);
}

void trap_handler_main(struct trapframe *esp){
	struct trapframe *tf = esp;
	print_tf(tf);

	if(tf -> trapno == 0x80){
		if(tf -> cs != 0x1b){
			kprintf("%x\n",tf -> cs);
			kprintf("syscall not from ring3\n");
			panic();
		}
		kprintf("inside syscall handler!");
		syscall_handler_main(tf->eax,tf->ebx,tf->ecx,tf->edx,tf->esi,tf->edi);
		panic();
	}
	if(tf -> trapno == 0x3){
		kprintf("hit brkpt. kernel halts\n");
		panic();
	}
}

