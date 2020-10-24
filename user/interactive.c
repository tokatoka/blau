#include "../lib/userheader.h"

int main(){
	syscall(0x2, 0, 1, 2, 3, 4);
	return 0;
}