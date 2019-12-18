#include "./fifo.h"
#include "./io.h"

void init_fifo32(struct fifo32 *f){
	for(int i = 0 ; i < 0x400 ; i++){
		f -> buf[i] = 0;
	}
	f -> p = 0;
	f -> q = 0;
	f -> size = 0;
}

void push_fifo32(struct fifo32 *f, unsigned int i){

	if(f -> size == 0x400){
		return;
	}

	if(f -> q == 0x400 - 1){
		f -> q = 0;
	}
	else{
		f -> q++;
	}

	f -> buf[f -> q] = i;
	f -> size++;
}

unsigned int pop_fifo32(struct fifo32 *f){
	if(f -> size == 0){
		return 0xdeadbeef;
	}

	if(f -> p == 0x400 - 1){
		f -> p = 0;
	}
	else{
		f -> p++;
	}

	unsigned int data = f -> buf[f -> p];
	f -> size--;
	return data;
}