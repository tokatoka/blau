#include "task.h"
#include "io.h"
extern struct task *current_task;

enum{
	SYSCALL_get_taskid = 0,
	SYSCALL_getc,
	SYSCALL_putc,
	SYSCALL_exit,
	SYSCALL_uptime,
};


unsigned int syscall_handler_main(unsigned int syscall_number,unsigned int erst,unsigned int zweit, unsigned int dritt, unsigned int viert,unsigned int fuenft){
	switch(syscall_number){
		case SYSCALL_get_taskid:
            return current_task->id;
            break;
		case SYSCALL_getc:
		case SYSCALL_putc:
			kprintf("%x\n",erst);
			return 0;
			break;
		case SYSCALL_exit:
		case SYSCALL_uptime:
		default:
			return -1;
	}
}