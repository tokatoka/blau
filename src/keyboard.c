#include "io.h"
#include "cpu.h"
#include "idt.h"
#include "fifo.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern struct fifo32 iobuf;

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