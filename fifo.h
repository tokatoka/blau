struct fifo32{
	unsigned int buf[0x400];
	unsigned int p;
	unsigned int q;
	unsigned int size;
};

void push_fifo32(struct fifo32 *,unsigned int);
unsigned int pop_fifo32(struct fifo32 *);
void init_fifo32(struct fifo32 *);