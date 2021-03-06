#ifndef BLAU_IO_H
#define BLAU_IO_H
void kprint(const char *str);
void kprint_newline(void);
void clear_screen(void);
void kput_char(char c);
void kint2hex(unsigned int);
void kint2dec(unsigned int);
void dump4bytes(char *);
void write4bytes(char *,unsigned int);
unsigned char kread_from_queue();
void interactive();
void kprintn(const char *str);
void kprintf(char *,...);
#endif