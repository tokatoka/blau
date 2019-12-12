build: kernel.c kernel.asm link.ld
	nasm -f elf32 kernel.asm -o kasm.o
	gcc -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c kernel.c -o kc.o
	gcc -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c printer.c -o printer.o
	gcc -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c timer.c -o timer.o
	ld -m elf_i386 -T link.ld -o blau kasm.o kc.o printer.o timer.o

run: blau
	qemu-system-i386 -kernel blau

debug: blau
	qemu-system-i386 -kernel blau -gdb tcp::6789

clean:
	rm -rf blau *.o