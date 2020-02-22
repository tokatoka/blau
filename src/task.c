#include "task.h"
#include "mem.h"
#include "util.h"
#include "io.h"
#include "cpu.h"
#include "elf.h"
#include "sched.h"

struct task *tasklist = 0;
struct task *current_task = 0;
struct task *freetasklist;
extern struct pde *master_pde;
extern unsigned int paging_enabled;
extern void load_master_pde();
extern void do_jump_user_function(unsigned int, unsigned int);


void task_init(){
	freetasklist = 0;
	for(int i = 0 ; i < MAX_TASKS; i++){
		tasklist[i].status = TASK_EMPTY;
		tasklist[i].id = 0;
		tasklist[i].next = freetasklist;
		freetasklist = &tasklist[i];
	}
}

unsigned int setup_task(struct task **store, unsigned int parent_id){

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
	t->pgdir[0].all = master_pde[0].all;


	pp->use++;

	t->pgdir[pde_idx((void *)TASKPGDIR)].off = (unsigned int)paddr(t->pgdir) / PGSIZE;
	t->pgdir[pde_idx((void *)TASKPGDIR)].present = 1;
	t->pgdir[pde_idx((void *)TASKPGDIR)].us = 1;


	t->id = ((char *)t - (char *)tasklist)/sizeof(struct task);
	t->parent_id = parent_id;
	t->status = TASK_READY;

	kmemset((char *)&t->tf,0,sizeof(t->tf));

	t -> tf.es = 0x20 | 3;
	t -> tf.ss = 0x20 | 3;
	t -> tf.cs = 0x18 | 3;
	t -> tf.ds = 0x20 | 3;
	t -> tf.esp = USTACK - 0x100;
	t -> tf.eflags |= 0x200; //allow interrupt
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
		unsigned int ret = page_insert(t->pgdir,pp,(void *)i,1,1);
		if(ret != 0){
			kprintf("mapping failed\n");
			panic();
		}
	}
}

unsigned int gen_task(void *bin){
	struct task *thistask;
	unsigned int ret = setup_task(&thistask,0);
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
			map_region(thistask,(void *)ph_start->p_vaddr,ph_start->p_memsz);
			char *test = (char *)0x800020;
			kmemcpy(ph_start->p_vaddr, (char *)bin + ph_start->p_offset, ph_start->p_filesz);
			kmemset((void *)ph_start -> p_vaddr + ph_start->p_filesz, 0, ph_start->p_memsz - ph_start->p_filesz);
		}
	}
	lcr3(master_pde);
	thistask->tf.eip = (unsigned int)elfhdr->e_entry;
	map_region(thistask,(void *)USTACK-PGSIZE,PGSIZE);
	return thistask->id;
}

void *id2task(unsigned int id){
	return &tasklist[id];
}

void task_destroy(struct task *t){
	lcr3(master_pde);
	for(unsigned int i = 0; i < pde_idx((void *)UTOP); i++){
		if(!(t -> pgdir[i].present == 1)){
			continue;
		}
		struct pte *victim_pte;
		void *pte_pa;
		pte_pa = (void *)(t->pgdir[i].off*PGSIZE);
		if(paging_enabled){
			victim_pte = kaddr(pte_pa);
		}
		for(unsigned int j = 0; j < 1024; j++){
			if(victim_pte->present == 1){
				page_remove(t->pgdir,(void *)pgaddr(i,j,0));
			}
		}
		t -> pgdir[i].all = 0;
		page_decref(pa2pp((void *)pte_pa));
	}
	void *pa = paddr(t -> pgdir);
	t -> pgdir = 0;
	page_decref(pa2pp(pa));
	t -> status = TASK_EMPTY;
	t -> next = freetasklist;
	freetasklist = t;
	if(t == current_task){
		current_task = 0;
		schedule();
	}

}


void run_task(struct task *t){
       if(t == 0){
               kprintf("invalid task specified\n");
       }
       if(current_task != t && current_task != 0){
               if(current_task->status == TASK_RUNNING){
                       current_task->status = TASK_READY;
               }
       }
       current_task = t;
       current_task->status = TASK_RUNNING;
       jump_user_function(t);
}


void jump_user_function(struct task *t){
	lcr3(paddr(t->pgdir));
	do_jump_user_function(t->tf.esp,t->tf.eip);
}
