#include "idt.h"
#include "io.h"
#include "timer.h"
#include "fifo.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#define ENTER_KEY_CODE 0x1C

unsigned int tick = 0;

extern unsigned char keyboard_map[128];

extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
extern void read_gdt(char *);

extern char* vidptr;
extern unsigned int current_loc;
char *gdt_entry;
struct fifo32 iobuf;


void allow_intr(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFC);
}

void timer_handler_main(void){
	write_port(0x20, 0x20);
	tick++;
}


void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;

	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0)
			return;

		push_fifo32(&iobuf,keycode);
	}
}


void disable_cursor()
{
	write_port(0x3D4, 0x0A);
	write_port(0x3D5, 0x20);
}

void kernel_panic()
{
	kprint("kernel panic!!! halting!");
	while(1);
}

int panic(){
	int a = 3 / 0;
}

void kmain(void)
{
	disable_cursor();
	clear_screen();
	kprint("blau kernel");
	kprint_newline();
	kprint_newline();

	idt_init();
	allow_intr();
	char addr[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
	read_gdt(addr);
	gdt_entry = (char *)*(unsigned int *)(addr + 2);


	kprint("checking gdt");
	kprint_newline();
	for(int i = 0 ; i < 2 * 5; i++){
		dump4bytes(gdt_entry + i * 4);
		kprint_newline();
	}


	interactive();
}