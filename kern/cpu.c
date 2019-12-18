#include "io.h"

extern void read_gdt(char *);
extern void haltloop();


void invlpg(void *va){
	__asm__ volatile("invlpg (%0)" : : "r" (va) : "memory");
}


void lldt(unsigned short sel){
	__asm__ volatile("lldt %0" : : "r" (sel));
}

unsigned int check_gdt(){
	char a = 0;
	read_gdt((char *)&a);
	return *(unsigned int *)(&a + 2);
}

void panic(){
	kprintf("kernel panic!");
	haltloop();
}