bits 32
section .text
global start
extern main
start:
	call main
	hlt