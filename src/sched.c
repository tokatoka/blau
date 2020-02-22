#include "task.h"
#include "cpu.h"
extern struct task* current_task;
extern struct task* tasklist;
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
	while(1){
		//halt
	}
	panic("no task available!\n");
}