#include "task.h"
#include "cpu.h"
#include "assert.h"
#include "io.h"
extern struct task* current_task;
extern struct task* tasklist;
extern struct pde *master_pde;
void schedule(){
	unsigned id = 0;
	if(current_task != 0){
		id = current_task->id;
	}
	for(int i = id + 1; i < MAX_TASKS ;i++){
		if(tasklist[i].status == TASK_READY){
			run_task(&tasklist[i]);
		}
	}
	for(int i = 0 ; i < id ; i++){
		if(tasklist[i].status == TASK_READY){
			run_task(&tasklist[i]);
		}
	}
	//halt
	assert(current_task == 0);
	lcr3(master_pde);
	__asm__ volatile("movl $0, %%ebp\n"
					 "movl %0, %%esp\n"
					 "pushl $0\n"
					 "pushl $0\n"
					 "sti\n"
					 "1:\n"
					 "hlt\n"
					 "jmp 1b\n"
				: : "a" (KSTACK)
		);

	panic("no task available!\n");
}
