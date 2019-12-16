unsigned int strlen(char *s);
unsigned char memcmp(char *a,char *b,unsigned int size);
void memcpy(char *src,char *dst,unsigned int size);
void memset(char *src,char v,unsigned int size);
void write4bytes(char *ptr, unsigned int v);
unsigned int read4bytes(char *);
unsigned int roundup(unsigned int num, unsigned int n);
unsigned int rounddown(unsigned int num, unsigned int n);