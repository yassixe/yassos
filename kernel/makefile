target=kernel.bin


all: boot.o kernel.o
	i686-elf-ld -T linker.ld boot.o kernel.o -o kernel.bin

boot.o: boot.s
	i686-elf-as boot.s -o boot.o

kernel.o: kernel.c
	i686-elf-gcc kernel.c -c -o kernel.o -Wall -Werror

clean:
	rm -rf *.o *.bin