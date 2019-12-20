#include "task.h"
#include "mem.h"
#include "util.h"
#include "io.h"
#include "cpu.h"
#include "elf.h"
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

unsigned int setup_new_tasks(struct task **store, unsigned int parent_id){

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


	t->id = (char *)t - (char *)tasklist;
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

unsigned int gen_task(void *bin){
	struct task *thistask;
	unsigned int ret = setup_new_tasks(&thistask,0);
	if(ret != 0){
		kprintf("gen_task failed\n");
		panic();
	}

	if(thistask == 0 | bin == 0){
		kprintf("gen_task failed\n");
		panic();
	}

	struct ELF *elfhdr = (struct ELF *)bin;
	if(elfhdr -> magic != ELFMAGIC){
		kprintf("%x",elfhdr);
		kprintf("invalid binary\n");
		panic();
	}

	struct Phdr *ph_start = (struct Phdr *)((char *)elfhdr + elfhdr->e_phoff);
	struct Phdr *ph_end = ph_start + elfhdr -> e_phnum;

	lcr3(paddr(thistask->pgdir));
	for(; ph_start < ph_end; ph_start++){
		if(ph_start->p_type == 1){ //load
			if(ph_start->p_memsz < ph_start->p_filesz){
				kprintf("invalid memsz\n");
				panic();
			}
			map_region(thistask,(void *)ph_start,ph_start->p_memsz);
			kmemcpy(ph_start->p_vaddr, (char *)bin + ph_start->p_offset, ph_start->p_filesz);
			kmemset((void *)ph_start -> p_vaddr + ph_start->p_filesz, 0, ph_start->p_memsz - ph_start->p_filesz);
		}
	}
	//lcr3(paddr(master_pde));
	thistask->tss.eip = (unsigned int)elfhdr->e_entry;
	map_region(thistask,(void *)USTACK-PGSIZE,PGSIZE);
	return thistask->id;
}

