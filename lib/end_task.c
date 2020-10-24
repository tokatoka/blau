#include "./userheader.h"

void end_task(){
	syscall(0x3, 0, 1, 2, 3, 4);
	return 0;
}