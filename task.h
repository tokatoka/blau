#ifndef BLAU_TASK_H
#define BLAU_TASK_H

#include "trap.h"
#include "mem.h"
#define MAX_TASKS 512

enum task_status{
	TASK_RUNNING,
	TASK_READY,
	TASK_ZOMBIE,
	TASK_EMPTY,
};

struct task{
	struct Trapframe tf;
	struct task *next;
	unsigned int id;
	struct pde* pgdir;
	unsigned int type;
	enum task_status status;
};

void task_init();
#endif