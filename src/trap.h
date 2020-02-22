#ifndef BLAU_TRAP_H
#define BLAU_TRAP_H

struct trapframe{
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int oesp;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;
	unsigned short es;
	unsigned short pad0;
	unsigned short ds;
	unsigned short pad1;
	unsigned int trapno;
	unsigned int errcode;
	unsigned int eip;
	unsigned short cs;
	unsigned short pad2;
	unsigned int eflags;
	unsigned int esp;
	unsigned short ss;
	unsigned short pad3;
};
#endif