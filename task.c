#include "task.h"
#include "mem.h"
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

	memset(&t->tss,0,sizeof(t->tss));

	*store = t;
	return 0;
}

void map_region(struct task *t, void *va, unsigned int len){
	
}