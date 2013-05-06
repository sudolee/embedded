
CC = arm-linux-gnueabi-gcc
LD = arm-linux-gnueabi-ld
OBJCOPY = arm-linux-gnueabi-objcopy
OBJDUMP = arm-linux-gnueabi-objdump

OPTIONS = -Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm \
		  -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float

all:
	${CC} ${OPTIONS} button-int.S -c
	${LD} -e _start -Ttext 0x0 button-int.o -o button-int.elf
	${OBJCOPY} -S -O binary button-int.elf button-int.bin
	${OBJDUMP} -D button-int.elf > bi.s

clean:
	rm -f button-int.o button-int.elf button-int.bin bi.s
