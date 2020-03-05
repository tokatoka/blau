#include "idt.h"
#include "io.h"
#include "timer.h"
#include "fifo.h"
#include "multiboot.h"
#include "mem.h"
#include "task.h"
#include "util.h"
#include "cpu.h"


#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#define ENTER_KEY_CODE 0x1C

unsigned int tick = 0;
unsigned int ext_max;

extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
extern void enable_paging();

extern unsigned int paging_enabled;
extern struct pde* master_pde;
extern char* vidptr;
extern unsigned int current_loc;
char *gdt_entry;
struct fifo32 iobuf;


void allow_intr(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFC);
}

void disable_cursor()
{
	write_port(0x3D4, 0x0A);
	write_port(0x3D5, 0x20);
}

void DBLFLT_handler_main()
{
	kprintf("double fault!!\n");
	panic();
}

void PGFLT_handler_main(unsigned int *addr){
	kprintf("page_fault at addr %x\n",addr);
	panic();
}

void test_mem(multiboot_info *info,unsigned int print){
	if(print){
		kprintf("beginning memory test\n");
		kprintf("lower: %x\n",info->mem_lower);
		kprintf("upper: %x\n",info->mem_upper);
	}


	for(memory_map *mmap = (memory_map *)info -> mmap_addr; (unsigned int)mmap < (info -> mmap_addr + info -> mmap_length); mmap++){
		if(print)kprintf("range %x %x, type %d\n",mmap -> base_addr_low, mmap -> length_low, mmap -> type);
		if(mmap -> base_addr_low == 0x100000){
			ext_max = mmap -> base_addr_low + mmap->length_low;
		}
	}
}

void kmain(unsigned long magic,multiboot_info *info)
{
	disable_cursor();
	clear_screen();
	kprintf("%s %s\n","blau","kernel");
	if(magic != MULTIBOOT_BOOTLOADER_MAGIC){
		kprintn("invalid multiboot magic!");
	}

	test_mem(info,0);
	idt_init();
	allow_intr();
	mem_init();
	enable_paging();
	paging_enabled = 1;
	task_init();
	unsigned int id = 0;
	GEN_AND_RUN_TASK(syscall);
	interactive();
}