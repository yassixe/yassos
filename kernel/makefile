target=kernel.bin


all: boot.o kernel.o mem.o
	i686-elf-ld -T linker.ld boot.o kernel.o mem.o -o kernel.bin

boot.o: boot.s
	i686-elf-as boot.s -o boot.o -g

kernel.o: kernel.c
	i686-elf-gcc kernel.c -c -o kernel.o -Wall -Werror -g
mem.o: mem.c
	i686-elf-gcc mem.c -c -o mem.o -Wall -Werror -g

clean:
	rm -rf *.o *.bin

run:
	qemu-system-i386 -kernel kernel.bin -M q35
debug:
	qemu-system-i386 -kernel kernel.bin -M q35 -s -S