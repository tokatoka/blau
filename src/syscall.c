#include "task.h"
#include "io.h"
extern struct task *current_task;

enum{
	SYSCALL_get_taskid = 0,
	SYSCALL_getc,
	SYSCALL_putc,
	SYSCALL_exit,
	SYSCALL_uptime,
	SYSCALL_getpid,
};


unsigned int syscall_handler_main(unsigned int syscall_number,unsigned int erst,unsigned int zweit, unsigned int dritt, unsigned int viert,unsigned int fuenft){
	switch(syscall_number){
			kprintf("%x\n",syscall_number);
		case SYSCALL_get_taskid:
            return current_task->id;
            break;
		case SYSCALL_getc:
		case SYSCALL_putc:
			kprintf("%x\n",erst);
			kprintf("%x\n",zweit);
			kprintf("%x\n",dritt);
			kprintf("%x\n",syscall_number);
			return 0;
			break;
		case SYSCALL_exit:
			task_destroy(current_task);
			break;
		case SYSCALL_uptime:
			break;
		case SYSCALL_getpid:

			break;
		default:
			return -1;
	}
}
