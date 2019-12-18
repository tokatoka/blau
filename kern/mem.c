#include "mem.h"
#include "util.h"
#include "io.h"
#include "assert.h"
#include "task.h"
#include "cpu.h"

#define PGSIZE 0x1000
#define NPDENTRIES 1024
#define NPTENTRIES 1024
#define PTSIZE (PGSIZE * NPTENTRIES)
#define KERNBASE 0xc0000000

char *next_free;
struct physpage *freeframelist;
struct physpage *framelist;
struct pde *master_pde;
unsigned int ext_max;
unsigned int paging_enabled = 0;
extern struct task* tasklist;

void *kaddr(void *va){
	return (void *)(KERNBASE + (unsigned int)va);
}

unsigned int pde_idx(void *va){
	return ((unsigned int)va >> 22) & 0x3ff;
}

unsigned int pte_idx(void *va){
	return ((unsigned int)va >> 12) & 0x3ff;
}

void *pp2pa(struct physpage *pp){
	return (void *)((pp - framelist) * 0x1000);
}

void *pp2kva(struct physpage *pp){
	return kaddr(pp2pa(pp));
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
	master_pde = (struct pde *)0x1ff000;
	struct pde* pde_p = master_pde;
	for(int i = 0; i < 0x400 ; i++){
		pde_p -> all = 0;
		pde_p++;
	}

	next_free = (void *)0x200000;
	freeframelist=0;
	framelist = (struct physpage *)alloc_mem((ext_max/0x1000) * sizeof(struct physpage));
	tasklist = (struct task *)alloc_mem(MAX_TASKS * sizeof(struct task));


	for(int i = 0; i < ext_max / PGSIZE ;i++){
		if(i < 0xa0000 / PGSIZE){
			framelist[i].use = 0;
			framelist[i].next = freeframelist;
			freeframelist = &framelist[i];
		}
		else if(i >= 0xa0000 && i < 0x400000){
			framelist[i].use = 1;
			continue;
		}
		else{
			framelist[i].use = 0;
			framelist[i].next = freeframelist;
			freeframelist = &framelist[i];
		}

	}



	//memorytest1();
	//memorytest2();


	boot_map_region(master_pde, (void *)0xa0000, (0x100000 - 0xa0000), (void *)0xa0000, 1, 0);
	//map IO hole
	boot_map_region(master_pde, (void *)0x100000, (0x400000 - 0x100000), (void *)0x100000, 1, 0);
	//map kern memory
	boot_map_region(master_pde, (void *)0xc0100000, (0xa000000 - 0x100000), (void *)0x100000,1,0);
	//map all physical memory upto 0x7fe0000

	//assert(ext_max == 0x7fe0000)
	//0x7fe0000 / 0x1000 = 32496


}

struct physpage *page_alloc(unsigned int zero){
	if(!freeframelist){
		return 0;
	}
	else{
		struct physpage *ret = freeframelist;
		if(zero){
			if(paging_enabled){
				kmemset((char *)pp2kva(ret),0,0x1000);
			}
			else{
				kmemset((char *)pp2pa(ret),0,0x1000);
			}
		}
		freeframelist = ret->next;
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
	pp -> next = freeframelist;
	freeframelist = pp;
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

struct pte* find_pte(struct pde* root, void *va,int create){
	struct pde *pde_p = &root[pde_idx(va)];
	struct pte *ret;
	if(pde_p -> present == 1){
		struct pte *pte_top = (void *)(pde_p -> off * PGSIZE);
		if(paging_enabled){
			pte_top = kaddr(pte_top);
		}
		ret = pte_top + pte_idx(va);
		return ret;
	}
	else if(!create){
		return 0;
	}
	else{
		struct physpage *new = page_alloc(1);
		if(!new){
			return 0;
		}
		else{
			new -> use++;
			root[pde_idx(va)].all = 0;
			root[pde_idx(va)].off = (unsigned int)pp2pa(new) / PGSIZE;
			root[pde_idx(va)].present = 1;
			root[pde_idx(va)].rw = 1;
			struct pte *pte_top = (struct pte *)pp2pa(new);
			if(paging_enabled){
				pte_top = kaddr(pte_top);
			}
			ret = pte_top + pte_idx(va);
			return ret;
		}
	}
}

struct physpage *page_lookup(struct pde *root, void *va, struct pte **pte_store){
	struct pte *pte_p = find_pte(root,va,0);
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


void tlb_invalidate(struct pde *root, void *va){
	invlpg(va);
}

void page_remove(struct pde *root, void *va){
	struct pte *pte_p;
	struct physpage *pi = page_lookup(root,va,&pte_p);
	if(!pi || !(pte_p -> present)){
		return;
	}
	page_decref(pi);
	tlb_invalidate(root,va);
	pte_p -> all = 0;
}


int page_insert(struct pde *root, struct physpage* pp, void *va, int rw,int us){
	struct pte * pte_p = find_pte(root,va,1);
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


void boot_map_region(struct pde *root, void * va, unsigned int size, void *pa, int rw,int us){
	for(int i = 0 ; i < size / PGSIZE ; i++){
		struct pte *pte_p = find_pte(root,va,1);
		if(!pte_p){
			kprintf("find_pte failed : %d!!!!\n",i);
			panic();
		}
		pte_p -> off = (unsigned int)pa / 0x1000;
		pte_p -> rw = rw;
		pte_p -> us = us;
		pte_p -> present = 1;
		va = va + PGSIZE;
		pa = pa + PGSIZE;
	}
}


unsigned int va2pa(struct pde *root,void *va){
	struct pte *p;
	root = &root[pde_idx(va)];
	if(!(root -> present)){
		return -1;
	}
	p = (struct pte *)(root->off * PGSIZE);
	if(paging_enabled){
		p = kaddr(p);
	}
	if(!p[pte_idx(va)].present){
		return -1;
	}
	return (unsigned int)(p[pte_idx(va)].off * PGSIZE + ((unsigned int)va & 0xfff));
}
