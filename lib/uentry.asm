bits 32
section .text
global start
extern main, end_task
start:
	call main
	call end_task
	;hlt