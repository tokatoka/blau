CC=gcc
LD=ld
CFLAGS=-m32 -fno-pie -no-pie -fno-builtin -static -fno-omit-frame-pointer -nostdlib
AS=nasm
ASFLAGS=-f elf32
PROGRAM=blau
KERN_SRCS=src/cpu.c src/fifo.c src/idt.c src/io.c src/kernel.c src/mem.c src/syscall.c src/task.c src/timer.c src/trap.c src/util.c src/sched.c src/keyboard.c
KERN_ASM_SRCS=src/kernel.asm
KERN_OBJS=src/cpu.o src/fifo.o src/idt.o src/io.o src/kernel.o src/mem.o src/syscall.o src/task.o src/timer.o src/trap.o src/util.o src/sched.o src/keyboard.o
KERN_ASM_OBJS=src/kasm.o
KERN_ASM_OBJS=src/kasm.o

LIB_SRCS=lib/brkpt.c lib/syscall.c
LIB_ASM_SRCS=lib/uentry.asm
LIB_OBJS=lib/brkpt.o lib/syscall.o
LIB_ASM_OBJS=lib/uentry.o

USER_SRCS=user/brkpt.c user/evil.c user/simple.c user/syscall.c
USER_TMP_OBJS=user/brkpt.tmp.o user/evil.tmp.o user/simple.tmp.o user/syscall.tmp.o
USER_OBJS=user/brkpt.o user/evil.o user/simple.o user/syscall.o

.PHONY: all
all: $(PROGRAM)
	qemu-system-i386 -kernel blau

$(KERN_ASM_OBJS): $(KERN_ASM_SRCS)
	$(AS) $(ASFLAGS) -o $@ $^

$(LIB_ASM_OBJS): $(LIB_ASM_SRCS)
	$(AS) $(ASFLAGS) -o $@ $^

$(KERN_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(USER_TMP_OBJS): %.tmp.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(USER_OBJS): %.o: %.tmp.o
	ld -m elf_i386 -T user/user.ld -o $@ $< $(LIB_OBJS)

$(PROGRAM) : $(KERN_ASM_OBJS) $(LIB_ASM_OBJS) $(LIB_OBJS) $(KERN_OBJS) $(USER_OBJS)
	ld -m elf_i386 -T src/link.ld -o $(PROGRAM) $(KERN_ASM_OBJS) $(KERN_OBJS) -b binary $(USER_OBJS)



.PHONY: run
run: $(PROGRAM)
	qemu-system-i386 -kernel blau

.PHONY: debug
debug: blau
	qemu-system-i386 -kernel blau -gdb tcp::6789 -S

.PHONY: monitor
monitor: blau
	qemu-system-i386 -kernel blau -monitor telnet:localhost:44444,server,nowait -enable-kvm

.PHONY: clean
clean:
	rm -rf blau src/*.o user/*.o lib/*.o
