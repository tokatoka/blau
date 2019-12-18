#include "task.h"

struct task *tasklist = 0;
struct task *current_task = 0;
struct task *freetasklist;

void task_init(){
	freetasklist = 0;
	for(int i = 0 ; i < MAX_TASKS; i++){
		tasklist[i].status = TASK_EMPTY;
		tasklist[i].id = 0;
		tasklist[i].next = freetasklist;
		freetasklist = &tasklist[i];
	}
}