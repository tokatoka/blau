#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES
#include "fifo.h"
#include "keyboard.h"
/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;
extern struct fifo32 iobuf;
extern unsigned char keyboard_map[128];

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}


void kput_char(char c){
	if(c == '\n'){
		kprint_newline();
	}
	else if(c == '\t'){
		current_loc += 8;
	}
	else{
		vidptr[current_loc++] = c;
		vidptr[current_loc++] = 0x07;
	}
}


void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		kput_char(str[i]);
		i++;
	}
}

void kint2dec(unsigned int i){
	unsigned int victim = 1000000000;
	char begin = 0;
	if(i == 0){
		kput_char('0');
		return;
	}
	for(int it = 1 ; it <= 10; it++){
		unsigned int value = i / victim;
		if(value == 0 && begin == 0){
			victim = victim / 10;
			continue;
		}
		else{
			begin = 1;
			kput_char('0' + value);
			i = i % victim;
			victim = victim / 10;
		}
	}
}


void kint2hex(unsigned int i){
	unsigned int victim = 0xf0000000;
	for(int it = 1 ; it <= 8 ; it++){
		unsigned int value = victim & i;
		value = value >> (32 - 4 * it);
		if(value <= 9){
			kput_char('0' + value);
		}
		else if(value == 10){
			kput_char('a');
		}
		else if(value == 11){
			kput_char('b');
		}
		else if(value == 12){
			kput_char('c');
		}
		else if(value == 13){
			kput_char('d');
		}
		else if(value == 14){
			kput_char('e');
		}
		else if(value == 15){
			kput_char('f');
		}
		else{
			kprint("error");
		}
		victim = (victim >> 4);
	}
}


unsigned char kread_from_queue(){
	unsigned int ret = pop_fifo32(&iobuf);

	if(ret == 0xdeadbeef){
		return 255;
	}
	else{
		return (unsigned) ret;
	}
}


void kprintn(const char *str){
	kprint(str);
	kprint_newline();
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
}

void dump4bytes(char *ptr){
	unsigned int value = *(unsigned int *)(ptr);
	kint2hex(value);
}


void kprintf(char *fmt,...){
	char **arg = (char **)&fmt;
	char c;
	arg++;

	while((c = *fmt++) != 0){
		if(c != '%'){
			kput_char(c);
		}
		else{
			c = *fmt++;
			switch(c){
				case 'd':
					if(0 > *((int *)arg)){
						kput_char('-');
						*((int *)arg) *= -1;
					}
					kint2dec(*((unsigned *) arg++));
					break;
				case 'x':
					kput_char('0');
					kput_char('x');
					kint2hex(*((unsigned *) arg++));
					break;
				case 's':
					kprint(*arg++);
					break;
				default:
					kput_char(c);
			}
		}
	}
}

void interactive(){
	while(1){
		unsigned char c = kread_from_queue();
		if(c == 255){
			continue;
		}
		else{
			if(keyboard_map[c] == 'q'){
				break;
			}
			kput_char(keyboard_map[c]);
		}
	}
}

