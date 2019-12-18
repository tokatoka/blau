#ifndef BLAU_MEM_H
#define BLAU_MEM_H

#define PGSIZE 0x1000
#define NPDENTRIES 1024
#define NPTENTRIES 1024
#define PTSIZE (PGSIZE * NPTENTRIES)
#define KERNBASE 0xc0000000
#define FRAMELIST_VA 0xbfc00000
#define TASKLIST_VA 0xbf800000


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
void boot_map_region(struct pde *root, void * va, unsigned int size, void *pa, int rw,int us);
unsigned int va2pa(struct pde *root,void *va);

#endif