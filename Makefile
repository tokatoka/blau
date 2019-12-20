CC=gcc

blau: kernel.c kernel.asm link.ld mem.c task.c
	nasm -f elf32 kernel.asm -o kasm.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c kernel.c -o kc.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c io.c -o io.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c timer.c -o timer.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c fifo.c -o fifo.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c idt.c -o idt.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c mem.c -o mem.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c util.c -o util.o
	$(CC) -m32 -fno-pie -masm=intel -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c task.c -o task.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c cpu.c -o cpu.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c syscall.c -o syscall.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c user/simple.c -o user/simple_tmp.o
	$(CC) -m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib -c user/evil.c -o user/evil_tmp.o
	ld -m elf_i386 -T user/user.ld -o user/simple.o user/simple_tmp.o
	ld -m elf_i386 -T user/user.ld -o user/evil.o user/evil_tmp.o
	ld -m elf_i386 -T link.ld -o blau kasm.o kc.o io.o timer.o fifo.o idt.o mem.o util.o syscall.o task.o cpu.o -b binary user/simple.o user/evil.o

run: blau
	qemu-system-i386 -kernel blau

debug: blau
	qemu-system-i386 -kernel blau -gdb tcp::6789 -S

monitor: blau
	qemu-system-i386 -kernel blau -monitor telnet:localhost:44444,server,nowait -enable-kvm

clean:
	rm -rf blau *.o user/*.o