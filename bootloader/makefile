all :
	mkdir -p build
	gcc loader.c                           \
      -c                                 \
      -fno-stack-protector               \
      -fpic                              \
      -fshort-wchar                      \
      -mno-red-zone                      \
      -I /usr/include/efi        \
      -I /usr/include/efi/x86_64 \
      -DEFI_FUNCTION_WRAPPER             \
      -o build/main.o

	ld build/main.o                         \
     /usr/lib/crt0-efi-x86_64.o     \
     -nostdlib                      \
     -znocombreloc                  \
     -T /usr/lib/elf_x86_64_efi.lds \
     -shared                        \
     -Bsymbolic                     \
     -L /usr/lib              \
     -l:libgnuefi.a                 \
     -l:libefi.a                    \
     -o build/main.so
 
	objcopy -j .text                \
          -j .sdata               \
          -j .data                \
          -j .dynamic             \
          -j .dynsym              \
          -j .rel                 \
          -j .rela                \
          -j .reloc               \
          --target=efi-app-x86_64 \
          build/main.so                 \
          build/main.efi

	dd if=/dev/zero of=build/uefi.img bs=512 count=93750
	parted build/uefi.img -s -a minimal mklabel gpt
	parted build/uefi.img -s -a minimal mkpart EFI FAT16 2048s 93716s
	parted build/uefi.img -s -a minimal toggle 1 boot
	dd if=/dev/zero of=/tmp/part.img bs=512 count=91669
	mformat -i /tmp/part.img -h 32 -t 32 -n 64 -c 1
	mcopy -i /tmp/part.img ./build/main.efi ::
	dd if=/tmp/part.img of=build/uefi.img bs=512 count=91669 seek=2048 conv=notrunc

qemu:
	sudo qemu-system-x86_64 -cpu qemu64 -bios /usr/share/ovmf/OVMF.fd -drive file=build/uefi.img,if=ide -k fr -net none

clean:
	rm -rf build/*