#define IOADR_PIT_COUNTER0	0x0040
#define IOADR_PIT_CONTROL_WORD	0x0043
#define IOADR_PIT_CONTROL_WORD_BIT_COUNTER0		0x00
#define IOADR_PIT_CONTROL_WORD_BIT_16BIT_READ_LOAD	0x30
#define IOADR_PIT_CONTROL_WORD_BIT_MODE2		0x04

void timer_init(void);