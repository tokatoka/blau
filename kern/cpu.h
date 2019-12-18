#ifndef BLAU_CPU_H
#define BLAU_CPU_H
unsigned int check_gdt();
void panic();
void invlpg(void *va);
void lldt(unsigned short sel);

#endif