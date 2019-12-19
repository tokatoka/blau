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

struct tss {
	unsigned int ts_link;
	unsigned int esp0;
	unsigned short ss0;
	unsigned short pad1;
	unsigned int esp1;
	unsigned short ss1;
	unsigned short pad2;
	unsigned int esp2;
	unsigned short ss2;
	unsigned short pad3;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned short es;
	unsigned short pad4;
	unsigned short cs;
	unsigned short pad5;
	unsigned short ss;
	unsigned short pad6;
	unsigned short ds;
	unsigned short pad7;
	unsigned short fs;
	unsigned short pad8;
	unsigned short gs;
	unsigned short pad9;
	unsigned short ldt;
	unsigned short pad10;
	unsigned short trace;
	unsigned short io;
};

struct task{
	struct tss tss;
	struct task *next;
	unsigned int id;
	unsigned int parent_id;
	struct pde* pgdir;
	unsigned int type;
	enum task_status status;
};

void task_init();
#endif