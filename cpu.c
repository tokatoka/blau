#include "io.h"

extern void read_gdt(char *);
extern void haltloop();

unsigned int check_gdt(){
	char a = 0;
	read_gdt((char *)&a);
	return *(unsigned int *)(&a + 2);
}

void panic(){
	kprintf("kernel panic!");
	haltloop();
}