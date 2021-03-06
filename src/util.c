#include "io.h"



unsigned int kstrlen(char *s){
	unsigned int ret;
	unsigned int i = 0;
	while(s[i] != '\x00'){
		i++;
		ret++;
	}
	return ret;
}

unsigned char kmemcmp(char *a,char *b,unsigned int size){
	for(int i = 0 ; i < size; i++){
		if(*(a + i) != *(b + i)){
			return 0;
		}
	}
	return 1;
}

void kmemcpy(char *dst,char *src,unsigned int size){
	for(int i = 0 ; i < size ; i++){
		*(dst + i) = *(src + i);
	}
}

void kmemset(char *src, char v, unsigned int size){
	for(int i = 0 ; i < size; i++){
		*(src + i) = v;
	}
}

void write4bytes(char *ptr, unsigned int v){
	unsigned int *lptr = (unsigned int *)ptr;
	*lptr = v;
}

unsigned int read4bytes(char *ptr){
	unsigned int *lptr = (unsigned int *)ptr;
	return *lptr;
}

unsigned int roundup(unsigned int num, unsigned int n){
	if(num % n == 0){
		return num;
	}
	else{
		unsigned int rem = num % n;
		return (num - rem) + n;
	}
}
unsigned int rounddown(unsigned int num,unsigned int n){
	if(num % n == 0){
		return num;
	}
	else{

		unsigned int rem = num % n;
		return (num - rem);
	}
}

