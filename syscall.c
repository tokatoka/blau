enum{
	SYSCALL_get_taskid,
	SYSCALL_getc,
	SYSCALL_putc,
	SYSCALL_exit,
	SYSCALL_uptime,
};


unsigned int syscall_handler_main(unsigned int syscall_number,unsigned int erst,unsigned int zweit, unsigned int dritt, unsigned int viert){
	switch(syscall_number){
		case SYSCALL_get_taskid:
		case SYSCALL_getc:
		case SYSCALL_putc:
		case SYSCALL_exit:
		case SYSCALL_uptime:
		default:
			return -1;
	}
}