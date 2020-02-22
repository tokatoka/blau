#ifndef BLAU_CPU_H
#define BLAU_CPU_H

struct gdt{
	struct{
		unsigned int limit_low : 16 __attribute__((packed));
		unsigned int base_low : 24 __attribute__((packed));
		unsigned int access : 1 __attribute__((packed));
		unsigned int rw : 1 __attribute__((packed));
		unsigned int dc : 1 __attribute__((packed));
		unsigned int ex : 1 __attribute__((packed));
		unsigned int s : 1 __attribute__((packed)) ;
		unsigned int dpl : 2 __attribute__((packed));
		unsigned int present : 1 __attribute__((packed));
		unsigned int limit_high : 4 __attribute__((packed));
		unsigned int avail : 1 __attribute__((packed));
		unsigned int zero : 1 __attribute__((packed));
		unsigned int big : 1 __attribute__((packed));
		unsigned int gran : 1 __attribute__((packed));
		unsigned int base_high : 8 __attribute__((packed));
	};
};

void *check_gdt();
void panic();
void brkpt();
void invlpg(void *va);
void write_tss();
void lcr3(void *val);
void lldt(unsigned short);
unsigned int rcr2(void);
unsigned int check_eflags();

#endif