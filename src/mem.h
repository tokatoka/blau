#ifndef BLAU_MEM_H
#define BLAU_MEM_H

#define PGSIZE 0x1000
#define NPDENTRIES 1024
#define NPTENTRIES 1024
#define PTSIZE (PGSIZE * NPTENTRIES)
#define KERNBASE 0xc0000000
#define FRAMELIST_VA 0xbfc00000
#define TASKLIST_VA 0xbf800000
#define TASKPGDIR 0xbf400000
#define UTOP 0xbf000000
#define USTACK 0xb0000000
#define KSTACK 0xc01f0000

struct pte{
	union{
		struct{
			unsigned int present : 1;
			unsigned int rw : 1;
			unsigned int us : 1;
			unsigned int wthru : 1;
			unsigned int cache : 1;
			unsigned int access : 1;
			unsigned int d : 1;
			unsigned int zero : 1;
			unsigned int g : 1;
			unsigned int avail : 3;
			unsigned int off : 20;
		};
		struct{
			unsigned int all;
		};
	};
};

struct pde{
	union{
		struct{
			unsigned int present : 1;
			unsigned int rw : 1;
			unsigned int us : 1;
			unsigned int wthru : 1;
			unsigned int cache : 1;
			unsigned int access : 1;
			unsigned int zero : 1;
			unsigned int size : 1;
			unsigned int ignored : 1;
			unsigned int avail : 3;
			unsigned int off : 20;
		};
		struct{
			unsigned int all;
		};
	};
};

struct physpage{
	struct physpage *next;
	unsigned char use;
};

void mem_init();
struct physpage *page_alloc();
void *pp2pa(struct physpage *);
void *pp2kva(struct physpage *pp);
void *kaddr(void *pa);
void boot_map_region(struct pde *root, void * va, unsigned int size, void *pa, int rw,int us);
unsigned int va2pa(struct pde *root,void *va);
unsigned int pde_idx(void *va);
unsigned int pte_idx(void *va);
void *pgaddr(unsigned int pdx, unsigned int ptx, unsigned int off);
int page_insert(struct pde *root, struct physpage* pp, void *va, int rw,int us);
void page_remove(struct pde *root, void *va);
void page_decref(struct physpage *pp);
struct physpage *pa2pp(void *pa);
void *paddr(void *va);
#endif