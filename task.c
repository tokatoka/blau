#include "task.h"
#include "mem.h"
#include "util.h"
#include "io.h"
#include "cpu.h"
struct task *tasklist = 0;
struct task *current_task = 0;
struct task *freetasklist;
extern struct pde *master_pde;
char ids[256];


void task_init(){
	freetasklist = 0;
	for(int i = 0 ; i < MAX_TASKS; i++){
		tasklist[i].status = TASK_EMPTY;
		tasklist[i].id = 0;
		tasklist[i].next = freetasklist;
		freetasklist = &tasklist[i];
	}
}

int fetch_id(){
	for(int i = 0 ; i < 256; i++){
		if(ids[i] == 0){
			ids[i] = 1;
			return i;
		}
	}
	return 0xdeadbeef;
}


unsigned int gen_new_tasks(struct task **store, unsigned int parent_id){

	if(freetasklist == 0){
		return -1;
	}
	struct task *t = freetasklist;
	freetasklist = t -> next;
	*store = t;

	struct physpage *pp = page_alloc(1);
	if(pp == 0){
		return -1;
	}
	t->pgdir = (struct pde *)pp2kva(pp);
	for(int i = 0 ; i < UTOP/PTSIZE; i++){
		t->pgdir[i].all = 0;
	}
	for(int i = UTOP/PTSIZE; i < 1024 ; i++){
		t->pgdir[i].all = master_pde[i].all;
	}
	pp->use++;

	t->pgdir[pde_idx((void *)TASKPGDIR)].off = (unsigned int)paddr(t->pgdir) / PGSIZE;
	t->pgdir[pde_idx((void *)TASKPGDIR)].present = 1;
	t->pgdir[pde_idx((void *)TASKPGDIR)].us = 1;


	t->id = fetch_id();
	t->parent_id = parent_id;
	t->status = TASK_READY;

	kmemset((char *)&t->tss,0,sizeof(t->tss));


	t -> tss.es = 0x20 | 3;
	t -> tss.ss = 0x20 | 3;
	t -> tss.cs = 0x18 | 3;
	t -> tss.ds = 0x20 | 3;
	t -> tss.esp = USTACK;
	t -> tss.eflags |= 0x200; //allow interrupt
	return 0;
}

void map_region(struct task *t, void *va, unsigned int len){
	void *start = (void *)rounddown((unsigned int)va, PGSIZE);
	void *end = (void *)roundup((unsigned int)va + len, PGSIZE);

	for(void *i = start; i < end; i+= PGSIZE){
		struct physpage *pp = page_alloc(0);
		if(pp == 0){
			kprintf("mapping failed\n");
			panic();
		}
		unsigned int ret = page_insert(t->pgdir,pp,(void *)i,0,1);
		if(ret != 0){
			kprintf("mapping failed\n");
			panic();
		}
	}
}