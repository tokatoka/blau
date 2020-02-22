unsigned int syscall(unsigned int syscall_number,unsigned int erst,unsigned int zweit, unsigned int dritt, unsigned int viert,unsigned int fuenft){
	int ret;
	__asm__ volatile("int %1\n"
		     : "=a" (ret)
		     : "i" (0x80),"a" (syscall_number),"d" (dritt),"c" (zweit),"b" (erst),"D" (fuenft),"S" (viert)
		     : "cc", "memory");
	return 1;
}