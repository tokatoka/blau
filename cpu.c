#include "io.h"
#include "cpu.h"
#include "task.h"
#include "mem.h"
#include "util.h"

extern void read_gdt(char *);
extern void haltloop();
extern unsigned int read_eflags();
struct tss master_tss;


void invlpg(void *va){
	__asm__ volatile("invlpg (%0)" : : "r" (va) : "memory");
}

void ltr(unsigned short sel)
{
	__asm__ volatile("ltr %0" : : "r" (sel));
}

void lldt(unsigned short sel)
{
	__asm__ volatile("lldt %0" : : "r" (sel));
}

void write_tss(struct gdt *g){
	unsigned int base = (unsigned int)&master_tss;
	unsigned int limit = sizeof(master_tss);
	g->limit_low=limit & 0xffff;
	g->base_low=base & 0xffffff;
	g->access=1;
	g->rw=0;
	g->dc=0;
	g->ex=1;
	g->s=0;
	g->dpl=3;
	g->present=1;
	g->limit_high=(limit & 0xf0000) >> 16;
	g->avail=0;
	g->zero=0;
	g->big=1;
	g->gran=0;
	g->base_high=(base & 0xff000000) >> 24;

	kmemset((char *)&master_tss,0,sizeof(master_tss));
	master_tss.ss0 = 0x10;
	master_tss.esp0=KSTACK;
	master_tss.io=sizeof(struct tss);


	ltr(0x2b);
}

void *check_gdt(){
	char a = 0;
	read_gdt((char *)&a);
	return (void *)*(unsigned int *)(&a + 2);
}

unsigned int check_eflags(){
	unsigned int a;
	a = read_eflags();
	return a;
}

void panic(){
	kprintf("kernel panic!");
	haltloop();
}