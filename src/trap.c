#include "task.h"
#include "io.h"
#include "syscall.h"
#include "cpu.h"
#include "assert.h"
#include "sched.h"

extern struct task* current_task;
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
	assert(!(check_eflags() & 0x200));
	struct trapframe *tf = esp;

	if((tf -> cs & 3) == 3){
		if(current_task->status == TASK_ZOMBIE){
			task_destroy(current_task);
			current_task = 0;
			schedule();
		}
		current_task -> tf = *tf;
		tf = &current_task -> tf;
	}

	if(tf -> trapno == 0x80){
		if(tf -> cs != 0x1b){
			kprintf("%x\n",tf -> cs);
			kprintf("syscall not from ring3\n");
			panic();
		}
		syscall_handler_main(tf->eax,tf->ebx,tf->ecx,tf->edx,tf->esi,tf->edi);
		panic();
	}
	if(tf -> trapno == 0x3){
		print_tf(esp);
		kprintf("hit brkpt. kernel halts\n");
		panic();
	}
	if(tf -> trapno == 0xd){
		print_tf(esp);
		kprintf("pgfault at %x\n",rcr2());
		panic();
	}
	if(tf -> trapno == 0x21){
		keyboard_handler_main();
	}
	else{
		print_tf(esp);
		kprintf("trap unhandled\n");
		panic();
	}
	if(current_task && current_task -> status == TASK_RUNNING){
		run_task(current_task);
	}
	else{
		schedule();
	}
}

