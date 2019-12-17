#include "mem.h"
#include "util.h"
#include "io.h"

#define PGSIZE 0x1000
char *next_free;
struct physpage *freelist;
struct physpage *framelist;


unsigned int pde_idx(void *va){
	return ((unsigned int)va >> 22) & 0x3ff;
}

unsigned int pte_idx(void *va){
	return ((unsigned int)va >> 12) & 0x3ff;
}

void *pp2pa(struct physpage *pp){
	return (void *)((pp - framelist) * 0x1000);
}

struct physpage *pa2pp(void *pa){
	return &framelist[((unsigned int)pa >> 12)];
}

void *alloc_mem(unsigned int n){
	unsigned int sz = roundup(n, 0x1000);
	char *result = next_free;
	next_free = next_free + sz;
	if(next_free > (char *)0x400000){
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


	next_free = (void *)0x200000;


	framelist = (struct physpage *)alloc_mem((0x7fe0000/0x1000) * sizeof(struct physpage));
	struct pte* pte_p = (struct pte *)alloc_mem(0x1000);
	master_pde -> all = 0;
	master_pde -> present = 1;
	master_pde -> rw = 1;
	master_pde -> off = ((unsigned int)pte_p / PGSIZE);

	unsigned int idx = 0;
	for(unsigned int i = 0 ; i < 0x400 ; i++){
		pte_p -> all = 0;
		pte_p -> present = 1;
		pte_p -> rw = 1;
		pte_p -> off = idx;
		pte_p++;
		idx++;
	}


	//0x7fe0000 / 0x1000 = 32496
	for(int i = 0; i < 0x7fe0000 / PGSIZE ;i++){
		if(i < 0x400000/PGSIZE){
			framelist[i].use = 1;
		}
		else{
			if(!freelist){
				freelist = &framelist[i];
			}
			framelist[i].use = 0;
			framelist[i].next = freelist;
			freelist = &framelist[i];
		}
	}

}

struct physpage *page_alloc(unsigned int zero){
	if(!freelist){
		return 0;
	}
	else{
		struct physpage *ret = freelist;
		if(zero){
			memset((char *)pp2pa(ret),0,0x1000);
		}
		freelist = ret->next;
		ret->next = 0;
		return ret;
	}
}

void page_free(struct physpage *pp){
	if(pp -> use != 0){
		kprintf("page still in use! \n");
		panic();
	}
	if(pp -> next != 0){
		kprintf("link not deleted! \n");
		panic();
	}
	pp -> next = freelist;
	freelist = pp;
}

void page_decref(struct physpage *pp){
	if(pp -> use == 0){
		kprintf("invalid decrement! \n");
		panic();
	}
	else{
		pp->use -= 1;
		if(pp->use == 0){
			page_free(pp);
		}
	}
}

struct pte* find_pte(struct pde* root, void *va){
	struct pde *pde_p = &root[pde_idx(va)];
	struct pte *ret;
	if(pde_p -> present == 1){
		struct pte *pte_top = (void *)(pde_p -> off * PGSIZE);
		ret = &pte_top[pte_idx(va)];
		return ret;
	}
	else{
		struct physpage *new = page_alloc(1);
		if(!new){
			return 0;
		}
		else{
			new -> use++;
			root[pde_idx(va)].all = 0;
			root[pde_idx(va)].off = (unsigned int)pp2pa(new);
			root[pde_idx(va)].present = 1;
			root[pde_idx(va)].rw = 1;
			struct pte *pte_top = (struct pte *)pp2pa(new);
			ret = &pte_top[pte_idx(va)];
			return ret;
		}
	}
}

struct physpage *page_lookup(struct pde *root, void *va, struct pte **pte_store){
	struct pte *pte_p = find_pte(root,va);
	if(pte_p != 0 && pte_p -> present){
		void *pa = (void *)(pte_p -> off * PGSIZE);
		struct physpage *ret = pa2pp(pa);
		if(pte_store){
			*pte_store = pte_p;
		}
		return ret;
	}
	else{
		return 0;
	}
}

void page_remove(struct pde *root, void *va){
	struct pte *pte_p;
	struct physpage *pi = page_lookup(root,va,&pte_p);
	if(!pi || !(pte_p -> present)){
		return;
	}
	page_decref(pi);
	pte_p -> all = 0;
}


int page_insert(struct pde *root, struct physpage* pp, void *va, int rw,int us){
	struct pte * pte_p = find_pte(root,va);
	if(pte_p == 0){
		return -1;
	}
	pp -> use++;
	if(pte_p -> present){
		page_remove(root,va);
	}
	pte_p -> off = (unsigned int)pp2pa(pp) / 0x1000;
	pte_p -> present = 1;
	pte_p -> rw = rw;
	pte_p -> us = us;
	root[pde_idx(va)].rw = rw;
	root[pde_idx(va)].us = us;
	return 0;
}
