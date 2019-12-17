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
void memorytest1();
void memorytest2();
void *pp2pa(struct physpage *);
void map_region(struct pde *root, void * va, unsigned int size, void *pa, int rw,int us);