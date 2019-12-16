#include "mem.h"
#include "util.h"
#include "io.h"

char *next_free;
void *alloc_mem(unsigned int n){
	unsigned int sz = roundup(n, 0x1000);
	char *result = next_free;
	next_free = next_free + sz;
	if(next_free > (char *)0x7ee0000){
		kprintf("out of memory!!");
	}
	return result;
}

void mem_init(){


	/*
	~0x100000 BIOS, display, etc...(only accessed by kernel)
	???? ~ 0x1f0000 kern_stack

	0x1ff000 master PDE

	~0x400000 kernel memory

	~0x7fe0000 all usable memory

	 */
	struct pde* master_pde = (struct pde *)0x1ff000;
	struct pde* pde_p = master_pde;
	for(int i = 0; i < 0x400 ; i++){
		pde_p -> all = 0;
		pde_p++;
	}


	master_pde -> all = 0;
	master_pde -> present = 1;
	master_pde -> rw = 1;
	master_pde -> off = 0x200;
	next_free = (void *)0x200000;


	struct pte* pte_p = (struct pte *)alloc_mem(0x1000);
	unsigned int idx = 0;
	for(unsigned int i = 0 ; i < 0x400 ; i++){
		pte_p -> all = 0;
		pte_p -> present = 1;
		pte_p -> rw = 1;
		pte_p -> off = idx;
		pte_p++;
		idx++;
	}

}

