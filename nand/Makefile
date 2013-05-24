CC = arm-linux-gnueabi-gcc
LD = arm-linux-gnueabi-ld
OBJCOPY = arm-linux-gnueabi-objcopy
OBJDUMP = arm-linux-gnueabi-objdump

OPTIONS = -Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm \
		  -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float \
		  -Wall -Iinc#-DDEBUG_LEDS

STACK_TOP := 0xFFF

all:
	${CC} ${OPTIONS} start.S -c -DSTACK_TOP=${STACK_TOP}
	${CC} ${OPTIONS} uart.c -c
	${CC} ${OPTIONS} clk.c -c
	${CC} ${OPTIONS} mem.c -c
	${CC} ${OPTIONS} print.c -c
	${CC} ${OPTIONS} str.c -c
	${CC} ${OPTIONS} nand.c -c
	${CC} ${OPTIONS} loadbin.c -c
	${CC} ${OPTIONS} platform.c -c
	${LD} -Tbuild.lds start.o clk.o mem.o uart.o str.o print.o nand.o platform.o loadbin.o -o Nand.elf
#	${LD} -Tbuild.lds start.o clk.o mem.o uart.o str.o         nand.o platform.o loadbin.o -o Nand.elf
	${OBJCOPY} -S -O binary Nand.elf Nand.bin
	${OBJDUMP} -D Nand.elf > n.S
	gcc -Wall usr/mkimage.c -o mkimage

clean:
	rm -f *.o *.elf *.bin n.S mkimage
