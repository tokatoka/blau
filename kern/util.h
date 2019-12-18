#ifndef BLAU_UTIL_H
#define BLAU_UTIL_H
unsigned int kstrlen(char *s);
unsigned char kmemcmp(char *a,char *b,unsigned int size);
void kmemcpy(char *src,char *dst,unsigned int size);
void kmemset(char *src,char v,unsigned int size);
void write4bytes(char *ptr, unsigned int v);
unsigned int read4bytes(char *);
unsigned int roundup(unsigned int num, unsigned int n);
unsigned int rounddown(unsigned int num, unsigned int n);
#endif