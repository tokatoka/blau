unsigned int strlen(char *s){
	unsigned int ret;
	unsigned int i = 0;
	while(s[i] != '\x00'){
		i++;
		ret++;
	}
	return ret;
}

unsigned char memcmp(char *a,char *b,unsigned int size){
	for(int i = 0 ; i < size; i++){
		if(*(a + i) != *(b + i)){
			return 0;
		}
	}
	return 1;
}

void memcpy(char *src,char *dst,unsigned int size){
	for(int i = 0 ; i < size ; i++){
		*(dst + i) = *(src + i);
	}
}

void memset(char *src, char v, unsigned int size){
	for(int i = 0 ; i < size; i++){
		*(src + i) = v;
	}
}