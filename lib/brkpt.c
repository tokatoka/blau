void brkpt(){
	__asm__ volatile("int %0\n"
		     :
		     : "i" (0x3)
		     : "cc");
}