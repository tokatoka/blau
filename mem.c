#include "mem.h"
#include "util.h"
#include "io.h"
#include "assert.h"

#define PGSIZE 0x1000
#define NPDENTRIES 1024
#define NPTENTRIES 1024
#define PTSIZE (PGSIZE * NPTENTRIES)
#define KERNBASE 0xc0000000

char *next_free;
struct physpage *freelist;
struct physpage *framelist;
struct pde *master_pde;
unsigned int ext_max;
unsigned int paging_enabled = 0;

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
	freelist=0;
	framelist = (struct physpage *)alloc_mem((ext_max/0x1000) * sizeof(struct physpage));



	for(int i = 0; i < ext_max / PGSIZE ;i++){
		if(i < 0xa0000 / PGSIZE){
			framelist[i].use = 0;
			framelist[i].next = freelist;
			freelist = &framelist[i];
		}
		else if(i >= 0xa0000 && i < 0x400000){
			framelist[i].use = 1;
			continue;
		}
		else{
			framelist[i].use = 0;
			framelist[i].next = freelist;
			freelist = &framelist[i];
		}

	}



	memorytest1();
	memorytest2();


	map_region(master_pde, 0xa0000, (0x100000 - 0xa0000), 0xa0000, 1, 0);
	//map IO hole
	map_region(master_pde, 0x100000, (0x400000 - 0x100000), 0x100000, 1, 0);
	//map kern memory
	map_region(master_pde, 0xc0100000, (0xa000000 - 0x100000), 0x100000,1,0);
	//map all physical memory upto 0x7fe0000

	//assert(ext_max == 0x7fe0000)
	//0x7fe0000 / 0x1000 = 32496


}

struct physpage *page_alloc(unsigned int zero){
	if(!freelist){
		return 0;
	}
	else{
		struct physpage *ret = freelist;
		if(zero){
			if(paging_enabled){
				memset((char *)pp2kva(ret),0,0x1000);
			}
			else{
				memset((char *)pp2pa(ret),0,0x1000);
			}
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


void invlpg(void *va){
	__asm__ volatile("invlpg (%0)" : : "r" (va) : "memory");
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


void map_region(struct pde *root, void * va, unsigned int size, void *pa, int rw,int us){
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
unsigned int check_vapa(struct pde *root,void *va){
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
	return p[pte_idx(va)].off * PGSIZE;
}

/*
test1 and test2 should be executed before enabling paging
test3 should be executed after enabling it.
 */

void memorytest1(){

	if(!freelist){
		panic();
	}
	struct physpage *pp, *pp0, *pp1, *pp2;
	unsigned int nfree;

	for(pp = freelist, nfree = 0;pp;pp=pp->next){
		nfree++;
	}
	pp0=pp1=pp2=0;
	assert((pp0 = page_alloc(0)));
	assert((pp1 = page_alloc(0)));
	assert((pp2 = page_alloc(0)));

	assert(pp0);
	assert(pp1 && pp1 != pp0);
	assert(pp2 && pp2 != pp1 && pp2 != pp0);

	struct physpage *fl;
	fl = freelist;
	freelist = 0;
	assert(!page_alloc(0));
	page_free(pp0);
	page_free(pp1);
	page_free(pp2);
	pp0 = pp1 = pp2 = 0;
	assert((pp0 = page_alloc(0)));
	assert((pp1 = page_alloc(0)));
	assert((pp2 = page_alloc(0)));
	assert(pp0);
	assert(pp1 && pp1 != pp0);
	assert(pp2 && pp2 != pp1 && pp2 != pp0);
	assert(!page_alloc(0));
	memset(pp2pa(pp0),1,PGSIZE);
	page_free(pp0);
	assert((pp = page_alloc(1)));
	assert(pp && pp0 == pp);
	char *c = pp2pa(pp);
	for(int i = 0 ; i < PGSIZE ; i++){
		assert(c[i] == 0);
	}

	freelist = fl;
	page_free(pp0);
	page_free(pp1);
	page_free(pp2);

	for (pp = freelist; pp; pp = pp->next)
		--nfree;
	assert(nfree == 0);
}
void memorytest2(){
	struct physpage *pp,*pp0,*pp1,*pp2;
	struct physpage *fl;
	struct pte *ptep,*ptep1;

	void *va;

	pp0 = pp1 = pp2 = 0;
	assert((pp0 = page_alloc(0)));
	assert((pp1 = page_alloc(0)));
	assert((pp2 = page_alloc(0)));

	assert(pp0);
	assert(pp1 && pp1 != pp0);
	assert(pp2 && pp2 != pp1 && pp2 != pp0);
	fl = freelist;
	freelist = 0;
	assert(!page_alloc(0));
	assert(page_lookup(master_pde, (void *)0, &ptep) == 0);
	assert(page_insert(master_pde, pp1, (void *)0, 1,0) < 0);
	page_free(pp0);
	assert(page_insert(master_pde, pp1, (void *)0, 1,0) == 0);
	assert(master_pde[0].off * PGSIZE == (unsigned int)pp2pa(pp0));
	assert(check_vapa(master_pde, 0) == pp2pa(pp1));
	assert(pp1->use == 1);
	assert(pp0->use == 1);

	assert(page_insert(master_pde, pp2, (void*) PGSIZE, 1,0) == 0);
	assert(check_vapa(master_pde, PGSIZE) == pp2pa(pp2));
	assert(pp2->use ==1);
	assert(!page_alloc(0));

	assert(page_insert(master_pde, pp2, (void*) PGSIZE, 1,0) == 0);
	assert(check_vapa(master_pde, PGSIZE) == pp2pa(pp2));
	assert(pp2->use ==1);

	assert(!page_alloc(0));

	ptep = (struct pte *)(master_pde[pde_idx(PGSIZE)].off * PGSIZE);
	assert(find_pte(master_pde,PGSIZE,0) == ptep + pte_idx(PGSIZE));

	assert(page_insert(master_pde, pp2, (void*) PGSIZE, 1,1) == 0);
	assert(check_vapa(master_pde, PGSIZE) == pp2pa(pp2));
	assert(pp2->use ==1);
	assert(find_pte(master_pde,PGSIZE,0)->us);
	assert(master_pde[0].us);

	assert(page_insert(master_pde, pp2, (void*) PGSIZE, 1,0) == 0);
	assert(find_pte(master_pde,PGSIZE,0)->rw);
	assert(!find_pte(master_pde,PGSIZE,0)->us);

	assert(page_insert(master_pde, pp0, (void*) PTSIZE, 1,0) < 0);

	assert(page_insert(master_pde, pp1, (void*) PGSIZE, 1,0) == 0);
	assert(!find_pte(master_pde,PGSIZE,0)->us);
	assert(check_vapa(master_pde, 0) == pp2pa(pp1));
	assert(check_vapa(master_pde, PGSIZE) == pp2pa(pp1));


	assert(pp1->use ==2);
	assert(pp2->use ==0);

	assert((pp = page_alloc(0)) && pp == pp2);
	page_remove(master_pde, 0x0);
	assert(check_vapa(master_pde, 0x0) == -1);
	assert(check_vapa(master_pde, PGSIZE) == pp2pa(pp1));
	assert(pp1->use ==1);
	assert(pp2->use ==0);

	assert(page_insert(master_pde, pp1, (void*) PGSIZE, 1,0) == 0);
	assert(pp1->use);
	assert(pp1->next == 0);

	page_remove(master_pde, PGSIZE);
	assert(check_vapa(master_pde,0) == -1);
	assert(check_vapa(master_pde,PGSIZE) == -1);
	assert(pp1->use == 0);
	assert(pp2->use == 0);

	assert((pp = page_alloc(0)) && pp == pp1);
	assert(!page_alloc(0));

	assert((void *)(master_pde[0].off * PGSIZE) == pp2pa(pp0));
	master_pde[0].all = 0;
	assert(pp0->use == 1);
	pp0 -> use = 0;
	page_free(pp0);
	va = (void*)(PGSIZE * NPDENTRIES + PGSIZE);
	ptep = find_pte(master_pde,va,1);
	ptep1 = (struct pte *)(master_pde[pde_idx(va)].off * PGSIZE);
	assert(ptep == ptep1 + pte_idx(va));
	master_pde[pde_idx(va)].all = 0;
	pp0 -> use = 0;

	memset(pp2pa(pp0),0xff,PGSIZE);
	page_free(pp0);
	find_pte(master_pde,0,1);
	ptep = (struct pte *)pp2pa(pp0);
	for(int i = 0 ; i < NPTENTRIES ; i++){
		assert(ptep[i].present == 0);
	}
	master_pde[0].all = 0;
	pp0->use = 0;
	freelist = fl;
	page_free(pp0);
	page_free(pp1);
	page_free(pp2);
}

void memorytest3(){
	struct physpage *pp,*pp0,*pp1,*pp2;
	struct physpage *fl;
	struct pte *ptep,*ptep1;

	void *va;
	pp1 = pp2 = 0;
	assert((pp0 = page_alloc(0)));
	assert((pp1 = page_alloc(0)));
	assert((pp2 = page_alloc(0)));
	page_free(pp0);

	memset(pp2kva(pp1),1,PGSIZE);
	memset(pp2kva(pp2),2,PGSIZE);

	page_insert(master_pde,pp1,PGSIZE,1,0);
	assert(pp1->use == 1);
	assert(*(unsigned int *)PGSIZE == 0x01010101U);


	page_insert(master_pde,pp2,PGSIZE,1,0);
	assert(*(unsigned int *)PGSIZE == 0x02020202U);
	assert(pp2->use == 1);
	assert(pp1->use == 0);
	*(unsigned int *)PGSIZE = 0x03030303U;

	assert(*(unsigned int *)pp2kva(pp2) == 0x03030303U);
	page_remove(master_pde,(void *)PGSIZE);
	kprintf("passed!\n");
	assert(pp2->use == 0);
	assert(master_pde[0].off * PGSIZE == pp2pa(pp0));
	master_pde[0].all = 0;
	assert(pp0->use == 1);
	pp0 -> use = 0;
	page_free(pp0);
}