unsigned int syscall(unsigned int syscall_number,unsigned int erst,unsigned int zweit, unsigned int dritt, unsigned int viert,unsigned int fuenft){
	int ret;
	__asm__ volatile("int %1\n"
		     : "=a" (ret)
		     : "i" (0x80),"a" (syscall_number),"d" (erst),"c" (zweit),"b" (dritt),"D" (viert),"S" (fuenft)
		     : "cc", "memory");
	return 1;
}