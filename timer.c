#include "timer.h"

extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

void timer_init(void)
{
    write_port(IOADR_PIT_CONTROL_WORD_BIT_COUNTER0
           | IOADR_PIT_CONTROL_WORD_BIT_16BIT_READ_LOAD
           | IOADR_PIT_CONTROL_WORD_BIT_MODE2, IOADR_PIT_CONTROL_WORD);
    write_port(0x9c, IOADR_PIT_COUNTER0);
    write_port(0x2e, IOADR_PIT_COUNTER0);
}
