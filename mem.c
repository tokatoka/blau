#include "mem.h"
#include "util.h"

void mem_init(){

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


	pde_p = master_pde + 0x300;
	pde_p -> all = 0;
	pde_p -> present = 1;
	pde_p -> rw = 1;
	pde_p -> off = 0x201;

	struct pte* pte_p = (struct pte *)0x200000;
	unsigned int idx = 0;
	for(unsigned int i = 0 ; i < 0x400 ; i++){
		pte_p -> all = 0;
		pte_p -> present = 1;
		pte_p -> rw = 1;
		pte_p -> off = idx;
		pte_p++;
		idx++;
	}
	pte_p = (struct pte *)0x201000;
	idx = 0;
	for(unsigned int i = 0 ; i < 0x400 ; i++){
		pte_p -> all = 0;
		pte_p -> present = 1;
		pte_p -> rw = 1;
		pte_p -> off = idx;
		pte_p++;
		idx++;
	}
}

