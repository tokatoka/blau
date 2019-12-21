#define IDT_SIZE 256
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define INTERRUPT_GATE_DPL0 0x8e
#define INTERRUPT_GATE_DPL3 0xee
#define TRAP_GATE_DPL0 0x8f

#define IOADR_PIT_COUNTER0	0x0040
#define IOADR_PIT_CONTROL_WORD	0x0043
#define IOADR_PIT_CONTROL_WORD_BIT_COUNTER0		0x00
#define IOADR_PIT_CONTROL_WORD_BIT_16BIT_READ_LOAD	0x30
#define IOADR_PIT_CONTROL_WORD_BIT_MODE2		0x04

#include "./io.h"
#include "timer.h"
#include "task.h"
#include "util.h"
#include "cpu.h"

extern void keyboard_handler(void);
extern void timer_handler(void);
extern void write_port(unsigned short port, unsigned char data);

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

extern void keyboard_handler(void);
extern void timer_handler(void);
extern void load_idt(unsigned long *idt_ptr);
extern void kprint(const char *);

extern void DIVIDE_handler(void);
extern void DEBUG_handler(void);
extern void NMI_handler(void);
extern void BPKPT_handler(void);
extern void OVLOW_handler(void);
extern void BOUND_handler(void);
extern void ILLOP_handler(void);
extern void DEVICE_handler(void);
extern void DBLFLT_handler(void);
extern void TSS_handler(void);
extern void SEGNP_handler(void);
extern void STACK_handler(void);
extern void GPFLT_handler(void);
extern void PGFLT_handler(void);
extern void FPERR_handler(void);
extern void ALIGN_handler(void);
extern void MCHK_handler(void);
extern void SIMDERR_handler(void);
extern void syscall_handler(void);



void set_idtdesc(unsigned int idx,void *handler_addr,unsigned short int sel,unsigned char type){
	unsigned long addr = (unsigned long)handler_addr;
	IDT[idx].offset_lowerbits = addr & 0xffff;
	IDT[idx].selector = sel;
	IDT[idx].zero = 0;
	IDT[idx].type_attr = type;
	IDT[idx].offset_higherbits = (addr & 0xffff0000) >> 16;
}


void idt_init(void)
{
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */

	set_idtdesc(0, DIVIDE_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(1, DEBUG_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(2, NMI_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(3, BPKPT_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL3);
	set_idtdesc(4, OVLOW_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(5, BOUND_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(6, ILLOP_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(7, DEVICE_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(8, DBLFLT_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(10, TSS_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(11, SEGNP_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(12, STACK_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(13, GPFLT_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(14, PGFLT_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(16, FPERR_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(17, ALIGN_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(18, MCHK_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);
	set_idtdesc(19, SIMDERR_handler, KERNEL_CODE_SEGMENT_OFFSET, INTERRUPT_GATE_DPL0);




	set_idtdesc(0x20,timer_handler,KERNEL_CODE_SEGMENT_OFFSET,INTERRUPT_GATE_DPL0);
	set_idtdesc(0x21,keyboard_handler,KERNEL_CODE_SEGMENT_OFFSET,INTERRUPT_GATE_DPL0);

	set_idtdesc(0x80,syscall_handler,KERNEL_CODE_SEGMENT_OFFSET,INTERRUPT_GATE_DPL3);

	timer_init();


	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;


	load_idt(idt_ptr);

	__asm__ volatile("movw %%ax,%%gs" :: "a" (0x20|3));
	__asm__ volatile("movw %%ax,%%fs" :: "a" (0x20|3));
	__asm__ volatile("movw %%ax,%%es" :: "a" (0x10));
	__asm__ volatile("movw %%ax,%%ds" :: "a" (0x10));
	__asm__ volatile("movw %%ax,%%ss" :: "a" (0x10));
	__asm__ volatile("ljmp %0,$1f\n 1:\n" :: "i" (0x08));

	lldt(0);
	struct gdt* gdt_arr= (struct gdt *)check_gdt();
	write_tss(&gdt_arr[5]);
}
