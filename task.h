#ifndef BLAU_TASK_H
#define BLAU_TASK_H

#include "trap.h"
#include "mem.h"
struct task{
	struct Trapframe tf;
	struct task *next;
	unsigned int id;
	struct pde* pgdir;
};
#endif