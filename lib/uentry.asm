bits 32
section .text
global _start
extern main
_start:
	call main
	hlt